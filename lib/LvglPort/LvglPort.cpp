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

// ---- LVGL draw buffers ----
static lv_disp_draw_buf_t draw_buf;
static lv_color_t* buf1 = nullptr;

// ---- Refresh flag ----
// LVGL flush renders into EPaper buffer; we call epaper.update() once per frame.
static volatile bool g_need_epaper_update = false;

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

  // Defer the physical E-Ink refresh to lvgl_port_loop() (once per frame).
  g_need_epaper_update = true;

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


  // Force initial EPaper refresh
  g_need_epaper_update = true;
}

void lvgl_port_loop() {
  // Run LVGL timers/animations
  lv_timer_handler();
  pagemgr_update();
  delay(5);

  // Commit one physical E-Ink refresh per “frame”
  if (g_need_epaper_update) {
    epaper.update();
    g_need_epaper_update = false;
  }
}

bool lvgl_port_needs_epaper_update() {
  return g_need_epaper_update;
}

void lvgl_port_clear_epaper_update_flag() {
  g_need_epaper_update = false;
}
