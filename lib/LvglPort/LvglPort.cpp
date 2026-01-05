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

static lv_obj_t* create_card(lv_obj_t* parent, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_pad_all(card, 10, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 4, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

static lv_obj_t* create_badge(lv_obj_t* parent, const char* txt) {
    lv_obj_t* lbl = lv_label_create(parent);
    lv_label_set_text(lbl, txt);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 0, 0);
    return lbl;
}


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

  
// ---- Dashboard layout (Step 7.2.2) ----
lv_obj_t* scr = lv_scr_act();
lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
lv_obj_set_style_pad_all(scr, 0, 0);
lv_obj_set_style_pad_gap(scr, 0, 0);

// =====================
// Row A – TOP CARDS
// =====================
lv_obj_t* row_top = lv_obj_create(scr);
lv_obj_set_size(row_top, 800, 240);
lv_obj_set_flex_flow(row_top, LV_FLEX_FLOW_ROW);
lv_obj_set_style_pad_gap(row_top, 0, 0);
lv_obj_clear_flag(row_top, LV_OBJ_FLAG_SCROLLABLE);

// ---- OUT card ----
lv_obj_t* card_out = create_card(row_top, 274, 240);
create_badge(card_out, "OUT Brottkarr");

lv_obj_t* out_temp = lv_label_create(card_out);
lv_label_set_text(out_temp, "-26.6 C");
lv_obj_align(out_temp, LV_ALIGN_TOP_LEFT, 0, 30);

lv_obj_t* out_meta = lv_label_create(card_out);
lv_label_set_text(out_meta,
    "Feels: -41 C\n"
    "RH: 11 %\n"
    "Press: 1099 hPa\n"
    "Wind: 15-24 m/s");
lv_obj_align(out_meta, LV_ALIGN_TOP_LEFT, 0, 80);

// ---- SENSOR card ----
lv_obj_t* card_sensor = create_card(row_top, 200, 240);
create_badge(card_sensor, "SENSOR In");

lv_obj_t* s1 = lv_label_create(card_sensor);
lv_label_set_text(s1, "19.4 C  RH 20 %");
lv_obj_align(s1, LV_ALIGN_TOP_LEFT, 0, 40);

lv_obj_t* s2 = lv_label_create(card_sensor);
lv_label_set_text(s2, "18.2 C  RH 28 %");
lv_obj_align(s2, LV_ALIGN_TOP_LEFT, 0, 80);

// ---- SKY card ----
lv_obj_t* card_sky = create_card(row_top, 200, 240);
create_badge(card_sky, "SKY Brottkarr");

lv_obj_t* sky_dummy = lv_label_create(card_sky);
lv_label_set_text(sky_dummy,
    "Sunrise/Sunset\n"
    "09:09\n"
    "09:45\n"
    "10:21\n"
    "...\n"
    "Day: 9h 39m");
lv_obj_align(sky_dummy, LV_ALIGN_TOP_LEFT, 0, 40);

// ---- INFO card ----
lv_obj_t* card_info = create_card(row_top, 104, 240);
create_badge(card_info, "INFO");

lv_obj_t* info_time = lv_label_create(card_info);
lv_label_set_text(info_time, "10:22");
lv_obj_align(info_time, LV_ALIGN_TOP_MID, 0, 40);

lv_obj_t* info_date = lv_label_create(card_info);
lv_label_set_text(info_date,
    "Sun 6.8\n"
    "Week 31\n"
    "2023\n"
    "DEV MODE");
lv_obj_align(info_date, LV_ALIGN_TOP_MID, 0, 80);

// =====================
// Row B – FORECAST
// =====================
lv_obj_t* row_fc = lv_obj_create(scr);
lv_obj_set_size(row_fc, 800, 240);
lv_obj_set_flex_flow(row_fc, LV_FLEX_FLOW_COLUMN);
lv_obj_set_style_pad_gap(row_fc, 6, 0);
lv_obj_clear_flag(row_fc, LV_OBJ_FLAG_SCROLLABLE);

// Header
lv_obj_t* fc_header = lv_label_create(row_fc);
lv_label_set_text(fc_header, "FORECAST Brottkarr");

// Cards row
lv_obj_t* fc_cards = lv_obj_create(row_fc);
lv_obj_set_flex_flow(fc_cards, LV_FLEX_FLOW_ROW);
lv_obj_set_style_pad_gap(fc_cards, 6, 0);
lv_obj_set_size(fc_cards, 800, 200);
lv_obj_clear_flag(fc_cards, LV_OBJ_FLAG_SCROLLABLE);

// Create 6 forecast cards
for (int i = 0; i < 6; i++) {
    lv_obj_t* fc = create_card(fc_cards, 120, 200);

    lv_obj_t* t = lv_label_create(fc);
    lv_label_set_text_fmt(t, "%02d:00", 15 + i * 3);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t* icon = lv_label_create(fc);
    lv_label_set_text(icon, "[ICON]");
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t* temp = lv_label_create(fc);
    lv_label_set_text(temp, "26 C");
    lv_obj_align(temp, LV_ALIGN_BOTTOM_MID, 0, -40);

    lv_obj_t* wind = lv_label_create(fc);
    lv_label_set_text(wind, "19 m/s");
    lv_obj_align(wind, LV_ALIGN_BOTTOM_MID, 0, -15);
}

  // Uppdatera UI var sekund
  lv_timer_create(ui_update_cb, 300000, nullptr); // 5 min 

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
