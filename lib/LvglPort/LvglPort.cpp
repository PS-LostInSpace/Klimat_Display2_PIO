#include "LvglPort.h"

#include <lvgl.h>
#include "driver.h"      // BOARD_SCREEN_COMBO 520
#include <TFT_eSPI.h>    // Seeed_GFX / EPaper types

#include "page1.h"       // Page 1 UI module (Step 8A)
#include "PageManager.h"
#include "fonts/kd2_fonts.h"
#include "kd2_mqtt_ui_bridge.h"

#ifndef KD2_USE_DUMMY_JSON
#define KD2_USE_DUMMY_JSON 0
#endif

#if KD2_USE_DUMMY_JSON
#include "kd2_dummy_data.h" // Optional: demo data for testing without MQTT
#endif


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
static const uint32_t WEATHER_STALE_MS = 6UL * 60UL * 1000UL;


#ifndef KD2_LVCONF_LOADED
#error "lv_conf.h is NOT being used by LVGL build!"
#endif

static ui_state_t g_state;
static bool g_prev_weather_alive_valid = false;
static bool g_prev_weather_alive = false;

static volatile bool g_has_mqtt_ui_msg = false;
static char g_ui_json_buf[1024];    // justera vid behov
static size_t g_ui_json_len = 0;

void lvgl_port_on_ui_json(const char* payload, size_t len) {
  Serial.printf("[LVGL_PORT] got json len=%u: %.*s\n", (unsigned)len, (int)len, payload);
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
  g_prev_weather_alive_valid = false;
  g_prev_weather_alive = false;

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

  pagemgr_begin(scr, &g_state);

// Force initial E-Ink refresh after first LVGL render
g_eink_refresh = EinkRefresh::Normal;
}

void lvgl_port_loop() {
  // ---------------------------------------------------------------------------
  // 0) Weather liveness edge-trigger (NO direct UI update here)
  // ---------------------------------------------------------------------------
  {
    const uint32_t now_ms = millis();
    const uint32_t last_mqtt_rx_ms = kd2_ui_last_mqtt_rx_ms();
    const bool have_rx = (last_mqtt_rx_ms != 0);

    bool is_alive = false;
    if (have_rx) {
      const uint32_t age_ms = now_ms - last_mqtt_rx_ms;
      is_alive = (age_ms < WEATHER_STALE_MS);
    }

    if (!g_prev_weather_alive_valid || (is_alive != g_prev_weather_alive)) {
      if (is_alive) {
        ui_state_set_updated(&g_state, 0);
      } else {
        ui_state_set_updated(&g_state, UINT16_MAX);
      }
      g_prev_weather_alive = is_alive;
      g_prev_weather_alive_valid = true;
    }
  }

  // ---------------------------------------------------------------------------
  // 1) Apply new UI data (JSON -> state) (NO direct UI update here)
  // ---------------------------------------------------------------------------
  if (g_has_mqtt_ui_msg) {
    g_has_mqtt_ui_msg = false;
    (void)kd2_ui_apply_mqtt_json(&g_state, g_ui_json_buf, g_ui_json_len);
    // IMPORTANT: Do NOT call page1_update() here.
    // PageManager will pick up g_state.dirty.any and update once.
  }

  // Optional: demo JSON injection (kept as-is)
#if KD2_USE_DUMMY_JSON
  static bool once = false;
  if (!once) {
    once = true;
    lvgl_port_on_ui_json(kd2_dummy_json(), kd2_dummy_json_len());
  }
#endif

  // ---------------------------------------------------------------------------
  // 2) Update UI exactly once via PageManager if anything is dirty
  // ---------------------------------------------------------------------------
  pagemgr_update();

  // ---------------------------------------------------------------------------
  // 3) Let LVGL process timers/flush after we've updated labels/objects
  // ---------------------------------------------------------------------------
  lv_timer_handler();
  delay(5);

  // ---------------------------------------------------------------------------
  // 4) E-Ink refresh policy: upgrade Normal -> Full sometimes
  // ---------------------------------------------------------------------------
  static uint32_t last_full_ms = 0;
  static uint16_t updates_since_full = 0;

  if (g_eink_refresh == EinkRefresh::Normal) {
    const uint32_t now = millis();

    const bool time_due =
        (last_full_ms == 0) || (now - last_full_ms >= EINK_FULL_REFRESH_MS);
    const bool count_due = (updates_since_full >= EINK_FULL_REFRESH_AFTER_N_UPDATES);

    if (time_due || count_due) {
      g_eink_refresh = EinkRefresh::Full;
    }
  }

  // ---------------------------------------------------------------------------
  // 5) Perform refresh when requested
  // ---------------------------------------------------------------------------
  if (g_eink_refresh != EinkRefresh::None) {
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
