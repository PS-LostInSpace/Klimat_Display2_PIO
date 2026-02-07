#include "LvglPort.h"

#include <lvgl.h>
#include "driver.h"      // BOARD_SCREEN_COMBO 520
#include <TFT_eSPI.h>    // Seeed_GFX / EPaper types

#include "page1.h"       // Page 1 UI module (Step 8A)
#include "PageManager.h"


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
  pagemgr_begin(scr);


// Force initial E-Ink refresh after first LVGL render
g_eink_refresh = EinkRefresh::Normal;

}

void lvgl_port_loop() {
  // Run LVGL timers/animations
  lv_timer_handler();
  pagemgr_update();
  delay(5);

  // ---------------------------------------------------------------------------
  // E-Ink refresh policy: refresh only when requested, force full refresh sometimes
  // ---------------------------------------------------------------------------
  static uint32_t last_full_ms = 0;
  static uint16_t updates_since_full = 0;

  // If we have a pending normal refresh, check if we should upgrade to FULL
  if (g_eink_refresh == EinkRefresh::Normal) {
    uint32_t now = millis();

    bool time_due = (last_full_ms == 0) || (now - last_full_ms >= EINK_FULL_REFRESH_MS);
    bool count_due = (updates_since_full >= EINK_FULL_REFRESH_AFTER_N_UPDATES);

    if (time_due || count_due) {
      g_eink_refresh = EinkRefresh::Full;
    }
  }

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
