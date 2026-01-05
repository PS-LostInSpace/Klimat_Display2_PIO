#include "LvglPort.h"
#include <Arduino.h>
#include <lvgl.h>
#include "driver.h"      // BOARD_SCREEN_COMBO 520
#include <TFT_eSPI.h>
#include "config.h"


#ifdef HAS_WIFI
  #include <WiFi.h>
#endif

// ---- Display (Seeed_GFX EPaper) ----
static EPaper epaper;

// ---- LVGL draw buffers ----
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = nullptr;

// ---- UI objects ----
static lv_obj_t *labelTitle = nullptr;
static lv_obj_t *labelClock = nullptr;
static lv_obj_t *labelWifi  = nullptr;

static volatile bool g_need_epaper_update = false;

// ---- Tick timer (1 ms) ----
static void lv_tick_cb(void *arg) {
  (void)arg;
  lv_tick_inc(1);
}

static inline uint16_t map_lv_color_to_epaper(lv_color_t c) {
  // LV_COLOR_DEPTH=8 => c.full is 0..255
  uint8_t v = c.full;
  return (v < 128) ? TFT_BLACK : TFT_WHITE;
}

static void my_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  // NOTE: Proof-of-life: drawPixel är långsamt men OK initialt
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      epaper.drawPixel(x, y, map_lv_color_to_epaper(*color_p));
      color_p++;
    }
  }

  // Vi gör själva update() i lvgl_port_loop() (en gång per “frame”)
  g_need_epaper_update = true;

  lv_disp_flush_ready(disp);
}

static void ui_update_cb(lv_timer_t *t) {
  (void)t;

  // ---- Clock (enkelt: uptime) ----
  uint32_t s = millis() / 1000;
  uint32_t hh = (s / 3600) % 24;
  uint32_t mm = (s / 60) % 60;
  uint32_t ss = s % 60;

  char bufClock[32];
  snprintf(bufClock, sizeof(bufClock), "Tid: %02lu:%02lu:%02lu", (unsigned long)hh, (unsigned long)mm, (unsigned long)ss);
  lv_label_set_text(labelClock, bufClock);

  // ---- WiFi ----
#ifdef HAS_WIFI
  if (WiFi.isConnected()) {
    String ip = WiFi.localIP().toString();
    int rssi = WiFi.RSSI();

    char bufWifi[64];
    snprintf(bufWifi, sizeof(bufWifi), "WiFi: OK  IP:%s  RSSI:%ddBm", ip.c_str(), rssi);
    lv_label_set_text(labelWifi, bufWifi);
  } else {
    lv_label_set_text(labelWifi, "WiFi: OFFLINE");
  }
#else
  lv_label_set_text(labelWifi, "WiFi: (disabled)");
#endif

  g_need_epaper_update = true;
}

void lvgl_port_begin() {
  // ---- EPaper init ----
  epaper.begin();
  epaper.fillScreen(TFT_WHITE);

  // ---- LVGL init ----
  lv_init();

  // 1ms tick via esp_timer
  const esp_timer_create_args_t tick_args = {
    .callback = &lv_tick_cb,
    .arg = nullptr,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "lv_tick"
  };
  esp_timer_handle_t tick_timer;
  esp_timer_create(&tick_args, &tick_timer);
  esp_timer_start_periodic(tick_timer, 1000); // 1000us = 1ms

  // Buffer: 800 * 40 lines
  const uint32_t buf_lines = 40;
  const uint32_t buf_px = 800 * buf_lines;

  // För LV_COLOR_DEPTH=8 räcker det ofta fint med PSRAM
  buf1 = (lv_color_t*)ps_malloc(buf_px * sizeof(lv_color_t));
  if (!buf1) {
    buf1 = (lv_color_t*)malloc(buf_px * sizeof(lv_color_t));
  }

  lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, buf_px);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 800;
  disp_drv.ver_res = 480;
  disp_drv.flush_cb = my_flush_cb;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.full_refresh = 1;   // ePaper: bra i början
  lv_disp_drv_register(&disp_drv);

  // ---- Dummy UI ----
  lv_obj_t *scr = lv_scr_act();

  labelTitle = lv_label_create(scr);
  lv_label_set_text(labelTitle, "Klimat_Display2 (E1001)");
  lv_obj_align(labelTitle, LV_ALIGN_TOP_MID, 0, 12);

  labelClock = lv_label_create(scr);
  lv_label_set_text(labelClock, "Tid: --:--:--");
  lv_obj_align(labelClock, LV_ALIGN_CENTER, 0, -10);

  labelWifi = lv_label_create(scr);
  lv_label_set_text(labelWifi, "WiFi: ...");
  lv_obj_align(labelWifi, LV_ALIGN_CENTER, 0, 20);

  // Uppdatera UI var sekund
  lv_timer_create(ui_update_cb, 60000, nullptr);

  // Första refresh
  g_need_epaper_update = true;
}

void lvgl_port_loop() {
  lv_timer_handler();
  delay(5);

  if (g_need_epaper_update) {
    epaper.update();   // Gör faktisk ePaper refresh här
    g_need_epaper_update = false;
  }
}

bool lvgl_port_needs_epaper_update() {
  return g_need_epaper_update;
}

void lvgl_port_clear_epaper_update_flag() {
  g_need_epaper_update = false;
}
