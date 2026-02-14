#include "LvglPort.h"

#include <lvgl.h>
#include "driver.h"      // BOARD_SCREEN_COMBO 520
#include <TFT_eSPI.h>    // Seeed_GFX / EPaper types

#include "page1.h"       // Page 1 UI module (Step 8A)
#include "PageManager.h"
#include "fonts/kd2_fonts.h"
#include "kd2_mqtt_ui_bridge.h"
#include "kd2_dummy_data.h" // Optional: demo data for testing without MQTT


// ============================================================================
// Display + LVGL port (E-Ink / EPaper)
// This file should only contain:
//   - EPaper driver instance
//   - LVGL display driver setup (buffers + flush callback)
//   - Main LVGL tick handling + EPaper update trigger
// UI/layout must live in ui modules (e.g. page1.cpp).
// ============================================================================

// ---- Display (Seeed_GFX EPaper) ----
static EPaper epaper;

// -----------------------------------------------------------------------------
// E-Ink refresh policy (Step 8D)
// -----------------------------------------------------------------------------
static const uint32_t EINK_FULL_REFRESH_MS = 15UL * 60UL * 1000UL; // every 15 minutes
static const uint16_t EINK_FULL_REFRESH_AFTER_N_UPDATES = 40;      // or after 40 UI updates


#ifndef KD2_LVCONF_LOADED
#error "lv_conf.h is NOT being used by LVGL build!"
#endif

static ui_state_t g_state;

static volatile bool g_has_mqtt_ui_msg = false;
static char g_ui_json_buf[512];     // justera vid behov
static size_t g_ui_json_len = 0;

void lvgl_port_on_ui_json(const char* payload, size_t len) {
  if(!payload || len == 0) return;
  if(len >= sizeof(g_ui_json_buf)) len = sizeof(g_ui_json_buf) - 1;

  memcpy(g_ui_json_buf, payload, len);
  g_ui_json_buf[len] = '\0';
  g_ui_json_len = len;
  g_has_mqtt_ui_msg = true;
}


// ---- LVGL draw buffers ----
static lv_disp_draw_buf_t draw_buf;
static lv_color_t* buf1 = nullptr;

// ---- Refresh flag ----
// Requested E-Ink refresh level (future-proof)
enum class EinkRefresh : uint8_t {
  None = 0,
  Normal,
  Full,
};

static volatile EinkRefresh g_eink_refresh = EinkRefresh::None;


// ----------------------------------------------------------------------------
// Pixel format mapping: LVGL (8-bit) -> EPaper (black/white)
// ----------------------------------------------------------------------------
static inline uint16_t map_lv_color_to_epaper(lv_color_t c) {
  // LV_COLOR_DEPTH=8 => c.full is 0..255 (luma-ish)
  const uint8_t v = c.full;
  return (v < 128) ? TFT_BLACK : TFT_WHITE;
}

// ----------------------------------------------------------------------------
// LVGL flush callback
// ----------------------------------------------------------------------------
static void my_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
  // NOTE: drawPixel per pixel is slow but OK for current step.
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      epaper.drawPixel(x, y, map_lv_color_to_epaper(*color_p));
      color_p++;
    }
  }

  // LVGL rendered something -> request a normal refresh
  g_eink_refresh = EinkRefresh::Normal;

  lv_disp_flush_ready(disp);
}

// ----------------------------------------------------------------------------
// Public API
// ----------------------------------------------------------------------------
void lvgl_port_begin() {
  // ---- EPaper init ----
  epaper.begin();
  epaper.fillScreen(TFT_WHITE);

  // ---- LVGL init ----
  lv_init();

  ui_state_init(&g_state);

  // ---- LVGL draw buffer ----
  // 800px wide * N lines (partial buffer)
  const uint32_t buf_lines = 40;
  const uint32_t buf_px = 800 * buf_lines;

  // Prefer PSRAM when available
  buf1 = (lv_color_t*)ps_malloc(buf_px * sizeof(lv_color_t));
  if (!buf1) {
    buf1 = (lv_color_t*)malloc(buf_px * sizeof(lv_color_t));
  }

  lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, buf_px);

  // ---- LVGL display driver ----
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  disp_drv.hor_res = 800;
  disp_drv.ver_res = 480;
  disp_drv.flush_cb = my_flush_cb;
  disp_drv.draw_buf = &draw_buf;

  // E-Ink: full refresh is OK for now (future: partial/dirty regions)
  disp_drv.full_refresh = 1;

  lv_disp_drv_register(&disp_drv);

  // ---- UI: Page 1 only (Step 8A) ----
  lv_obj_t* scr = lv_scr_act();
  lv_obj_set_style_text_font(scr, UI_FONT_BODY, LV_PART_MAIN);

  kd2_ui_apply_mqtt_json(&g_state, kd2_dummy_json(), kd2_dummy_json_len()); // kommentera för att ta bort demo injection
  pagemgr_begin(scr, &g_state);

// Force initial E-Ink refresh after first LVGL render
g_eink_refresh = EinkRefresh::Normal;
}

void lvgl_port_loop() {
  // ---------------------------------------------------------------------------
  // 1) Run LVGL timers/animations + page manager
  // ---------------------------------------------------------------------------
  lv_timer_handler();
  pagemgr_update();

  static uint32_t last_minute = 0;
  if(millis() - last_minute >= 60000) {
    last_minute = millis();
    ui_state_set_updated(&g_state, g_state.updated_min_ago + 1);
    page1_update(&g_state);
    ui_state_clear_dirty(&g_state);
    g_eink_refresh = EinkRefresh::Normal;
  }

  delay(5);

  // ---------------------------------------------------------------------------
  // 2) Apply new UI data (JSON handoff) -> request a normal refresh
  // ---------------------------------------------------------------------------
  if (g_has_mqtt_ui_msg) {
    g_has_mqtt_ui_msg = false;

    if (kd2_ui_apply_mqtt_json(&g_state, g_ui_json_buf, g_ui_json_len)) {
      page1_update(&g_state);
      ui_state_clear_dirty(&g_state);

      // New data arrived -> request refresh
      g_eink_refresh = EinkRefresh::Normal;
    }
  }

  // ---------------------------------------------------------------------------
  // 3) E-Ink refresh policy: upgrade Normal -> Full sometimes
  // ---------------------------------------------------------------------------
  static uint32_t last_full_ms = 0;
  static uint16_t updates_since_full = 0;

  if (g_eink_refresh == EinkRefresh::Normal) {
    const uint32_t now = millis();

    const bool time_due  = (last_full_ms == 0) || (now - last_full_ms >= EINK_FULL_REFRESH_MS);
    const bool count_due = (updates_since_full >= EINK_FULL_REFRESH_AFTER_N_UPDATES);

    if (time_due || count_due) {
      g_eink_refresh = EinkRefresh::Full;
    }
  }

  // ---------------------------------------------------------------------------
  // 4) Perform refresh when requested
  // ---------------------------------------------------------------------------
  if (g_eink_refresh != EinkRefresh::None) {
    // NOTE: With current Seeed_GFX driver, update() is a full-panel refresh anyway.
    // Later we can map Normal->partial and Full->full.
    epaper.update();

    if (g_eink_refresh == EinkRefresh::Full) {
      last_full_ms = millis();
      updates_since_full = 0;
    } else {
      updates_since_full++;
    }

    g_eink_refresh = EinkRefresh::None;
  }
}


bool lvgl_port_needs_epaper_update() {
  return (g_eink_refresh != EinkRefresh::None);
}

void lvgl_port_clear_epaper_update_flag() {
  g_eink_refresh = EinkRefresh::None;
}
