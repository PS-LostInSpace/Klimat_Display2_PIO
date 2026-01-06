#include "LvglPort.h"
#include <Arduino.h>
#include <lvgl.h>
#include "driver.h"      // BOARD_SCREEN_COMBO 520
#include <TFT_eSPI.h>

#ifdef HAS_WIFI
  #include <WiFi.h>
#endif

// ---- Display (Seeed_GFX EPaper) ----
static EPaper epaper;

// ---- LVGL draw buffers ----
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = nullptr;

// ---- UI objects ----
// static lv_obj_t *labelTitle = nullptr;
// static lv_obj_t *labelClock = nullptr;
// static lv_obj_t *labelWifi  = nullptr;

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

// static void ui_update_cb(lv_timer_t *t) {
//   (void)t;

//   // ---- Clock (enkelt: uptime) ----
//   uint32_t s = millis() / 1000;
//   uint32_t hh = (s / 3600) % 24;
//   uint32_t mm = (s / 60) % 60;
//   uint32_t ss = s % 60;

//   char bufClock[32];
//   snprintf(bufClock, sizeof(bufClock), "Tid: %02lu:%02lu:%02lu", (unsigned long)hh, (unsigned long)mm, (unsigned long)ss);
//   lv_label_set_text(labelClock, bufClock);

//   // ---- WiFi ----
// #ifdef HAS_WIFI
//   if (WiFi.isConnected()) {
//     String ip = WiFi.localIP().toString();
//     int rssi = WiFi.RSSI();

//     char bufWifi[64];
//     snprintf(bufWifi, sizeof(bufWifi), "WiFi: OK  IP:%s  RSSI:%ddBm", ip.c_str(), rssi);
//     lv_label_set_text(labelWifi, bufWifi);
//   } else {
//     lv_label_set_text(labelWifi, "WiFi: OFFLINE");
//   }
// #else
//   lv_label_set_text(labelWifi, "WiFi: (disabled)");
// #endif

//   g_need_epaper_update = true;
// }

// -----------------
// Page 1 (single) UI
// -----------------

static lv_obj_t* create_box(lv_obj_t* parent, lv_coord_t w, lv_coord_t h, bool border = true) {
  lv_obj_t* o = lv_obj_create(parent);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_pad_all(o, 0, 0);
  lv_obj_set_style_pad_gap(o, 0, 0);
  lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(o, border ? 1 : 0, 0);
  lv_obj_set_style_border_color(o, lv_color_black(), 0);
  return o;
}

static lv_obj_t* create_label(lv_obj_t* parent, const char* txt) {
  lv_obj_t* l = lv_label_create(parent);
  lv_label_set_text(l, txt);
  return l;
}

// Bygger layouten direkt på scr (ingen page manager ännu)
static void ui_build_page1(lv_obj_t* parent) {
  // Nollställ lite styling så vi får full kontroll över pixelbredden
  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(parent, 10, 0);
  lv_obj_set_style_pad_gap(parent, 0, 0);

  // Root row: 3 columns
  lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_gap(parent, 0, 0);

  const lv_coord_t W_LEFT  = 270;
  const lv_coord_t W_MID   = 260;
  const lv_coord_t W_RIGHT = 270;
  const lv_coord_t H = 480 - 20; // parent padding 10+10

  lv_obj_t* col_left  = create_box(parent, W_LEFT,  H, false);
  lv_obj_t* sep1      = create_box(parent, 1,       H, false);
  lv_obj_t* col_mid   = create_box(parent, W_MID,   H, false);
  lv_obj_t* sep2      = create_box(parent, 1,       H, false);
  lv_obj_t* col_right = create_box(parent, W_RIGHT, H, false);

  // Separator lines
  lv_obj_set_style_bg_opa(sep1, LV_OPA_100, 0);
  lv_obj_set_style_bg_color(sep1, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(sep2, LV_OPA_100, 0);
  lv_obj_set_style_bg_color(sep2, lv_color_black(), 0);

  // LEFT: NU + vind
  lv_obj_set_style_pad_all(col_left, 14, 0);

  lv_obj_t* lbl_nu = create_label(col_left, "NU");
  lv_obj_align(lbl_nu, LV_ALIGN_TOP_LEFT, 0, 0);

  lv_obj_t* compass = create_box(col_left, 140, 140, true);
  lv_obj_align(compass, LV_ALIGN_TOP_LEFT, 0, 50);
  lv_obj_t* lbl_comp = create_label(compass, "NO");
  lv_obj_align(lbl_comp, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t* lbl_dir = create_label(col_left, "NO");
  lv_obj_align(lbl_dir, LV_ALIGN_TOP_LEFT, 160, 80);

  lv_obj_t* lbl_ms = create_label(col_left, "5.1 m/s");
  lv_obj_align(lbl_ms, LV_ALIGN_TOP_LEFT, 160, 120);

  lv_obj_t* line = create_box(col_left, W_LEFT - 28, 1, false);
  lv_obj_set_style_bg_opa(line, LV_OPA_100, 0);
  lv_obj_set_style_bg_color(line, lv_color_black(), 0);
  lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, 240);

  lv_obj_t* lbl_om = create_label(col_left, "OM 1-2 h");
  lv_obj_align(lbl_om, LV_ALIGN_TOP_LEFT, 0, 200);

  lv_obj_t* lbl_future = create_label(col_left,
    "15.3 m/s  ONO\n"
    "11.4 m/s  NNV");
  lv_obj_align(lbl_future, LV_ALIGN_TOP_LEFT, 0, 260);

  // MID: ikon + regn 90
  lv_obj_set_style_pad_all(col_mid, 14, 0);

  lv_obj_t* wx_icon = create_box(col_mid, W_MID - 28, 140, true);
  lv_obj_align(wx_icon, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_t* lbl_wx = create_label(wx_icon, "[WX ICON]");
  lv_obj_align(lbl_wx, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t* bars = create_box(col_mid, W_MID - 28, 220, false);
  lv_obj_align(bars, LV_ALIGN_BOTTOM_MID, 0, -10);

  struct Bar { int h; const char* t; } b[3] = {
    { 50,  "30"     },
    { 110, "60"     },
    { 170, "90 min" }
  };

  for (int i = 0; i < 3; i++) {
    lv_obj_t* bar = lv_obj_create(bars);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_color(bar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(bar, lv_color_black(), 0);

    lv_obj_set_size(bar, 60, b[i].h);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_LEFT, i * 80, -30);

    lv_obj_t* t = create_label(bars, b[i].t);
    lv_obj_align(t, LV_ALIGN_BOTTOM_LEFT, i * 80 + 10, 0);
  }

  // RIGHT: UTE/INNE + Atmosfar
  lv_obj_set_style_pad_all(col_right, 14, 0);

  lv_obj_t* lbl_ute = create_label(col_right, "UTE");
  lv_obj_align(lbl_ute, LV_ALIGN_TOP_LEFT, 0, 0);

  lv_obj_t* ute_val = create_label(col_right, "-22.3 C");
  lv_obj_align(ute_val, LV_ALIGN_TOP_LEFT, 0, 40);

  lv_obj_t* lbl_inne = create_label(col_right, "INNE");
  lv_obj_align(lbl_inne, LV_ALIGN_TOP_LEFT, 0, 140);

  lv_obj_t* inne_val = create_label(col_right, "24.2 C");
  lv_obj_align(inne_val, LV_ALIGN_TOP_LEFT, 0, 180);

  lv_obj_t* line2 = create_box(col_right, W_RIGHT - 28, 1, false);
  lv_obj_set_style_bg_opa(line2, LV_OPA_100, 0);
  lv_obj_set_style_bg_color(line2, lv_color_black(), 0);
  lv_obj_align(line2, LV_ALIGN_TOP_LEFT, 0, 260);

  lv_obj_t* lbl_atm = create_label(col_right, "Atmosfar");
  lv_obj_align(lbl_atm, LV_ALIGN_TOP_LEFT, 0, 280);

  lv_obj_t* atm_vals = create_label(col_right,
    "Tryck: 1000.3 mbar\n"
    "Fukt:  90 %");
  lv_obj_align(atm_vals, LV_ALIGN_TOP_LEFT, 0, 320);

  lv_obj_t* updated = create_label(col_right, "Uppdaterat: 1 h sedan");
  lv_obj_align(updated, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
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


  // ---- Step 7 (Step 7 page1-only) ----
  lv_obj_t *scr = lv_scr_act();
  ui_build_page1(scr);

  // Tillfälligt: stäng av gamla uppdateringstimern (den använder labelClock/labelWifi)
  // lv_timer_create(ui_update_cb, 1000, nullptr);

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
