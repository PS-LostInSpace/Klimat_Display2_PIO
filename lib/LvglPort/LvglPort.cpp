#include "LvglPort.h"
// #include <Arduino.h>
#include <lvgl.h>
#include "driver.h"      // BOARD_SCREEN_COMBO 520
#include <TFT_eSPI.h>
#include "page1.h"


// ---- Display (Seeed_GFX EPaper) ----
static EPaper epaper;

// ---- LVGL draw buffers ----
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = nullptr;

// ---- UI objects ----
static volatile bool g_need_epaper_update = false;

// ---- Page1 rain bars ----
static lv_obj_t* g_rain_bar[3] = {nullptr, nullptr, nullptr};
static lv_obj_t* g_rain_pct[3] = {nullptr, nullptr, nullptr};
static lv_obj_t* g_rain_col[3] = {nullptr, nullptr, nullptr}; 
static const char* g_rain_t[3] = {"30", "60", "90 min"};

static lv_obj_t* g_rain_base[3] = {nullptr, nullptr, nullptr};


// ---- Page 1 rain layout tuning ----
static const lv_coord_t Y_RAIN_PERCENT = -8;  // y-offset for percent label inside each bar column
static const lv_coord_t Y_RAIN_XLABEL  = 4;   // y-offset for 30/60/90 labels (downwards)
static const lv_coord_t Y_RAIN_BAR_BASE_Y = -20; // baseline/bar "ground" offset from bottom
static const lv_coord_t PLOT_H = 112;         // max bar height in pixels (headroom vs % label)


static int clamp_int(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// ---- Page1 API (future-proof) ----
static void page1_set_rain_probs(int p30, int p60, int p90) {
  const int p[3] = {
    clamp_int(p30, 0, 100),
    clamp_int(p60, 0, 100),
    clamp_int(p90, 0, 100)
  };

  const lv_coord_t PLOT_H = 112; // Max bar height in pixels 
  const lv_coord_t MIN_H = 2;

  for (int i = 0; i < 3; i++) {
    if (!g_rain_bar[i]) continue;

    lv_coord_t h = (lv_coord_t)((p[i] * PLOT_H) / 100);
    if (h < MIN_H) h = MIN_H;

    lv_obj_set_height(g_rain_bar[i], h);
    lv_obj_align(g_rain_bar[i], LV_ALIGN_BOTTOM_MID, 0, Y_RAIN_BAR_BASE_Y);

    if (g_rain_pct[i]) {
      // Keep percent label position consistent (even when bars update)
      lv_obj_align(g_rain_pct[i], LV_ALIGN_TOP_MID, 0, Y_RAIN_PERCENT);
      
      char buf[8];
      snprintf(buf, sizeof(buf), "%d%%", p[i]);
      lv_label_set_text(g_rain_pct[i], buf);

      lv_obj_move_foreground(g_rain_pct[i]);

    }
  }

  g_need_epaper_update = true;
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

static void apply_text_style_title(lv_obj_t* lbl) {
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(lbl, lv_color_black(), 0);
}

static void apply_text_style_value(lv_obj_t* lbl) {
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(lbl, lv_color_black(), 0);
}

// ============================================================================
// PAGE 1 LAYOUT
// Three-column dashboard:
//   LEFT  = Wind (current + forecast)
//   MID   = Weather icon + rain probability bars
//   RIGHT = Indoor/Outdoor + atmosphere
// ============================================================================

static void ui_build_page1(lv_obj_t* parent) {

  // --------------------------------------------------------------------------
  // ROOT CONTAINER SETUP
  // --------------------------------------------------------------------------
  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(parent, 10, 0);
  lv_obj_set_style_pad_gap(parent, 0, 0);

  lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_gap(parent, 0, 0);

  // Column widths
  const lv_coord_t W_LEFT  = 270;
  const lv_coord_t W_MID   = 260;
  const lv_coord_t W_RIGHT = 270;
  const lv_coord_t H = 480 - 20;

  // Shared vertical rhythm (Step 7A)
  const lv_coord_t Y_SECTION_DIV     = 220;
  const lv_coord_t Y_SECTION_TITLE   = Y_SECTION_DIV - 18;
  const lv_coord_t Y_SECTION_CONTENT = Y_SECTION_DIV + 18;

  // --------------------------------------------------------------------------
  // COLUMN CONTAINERS + SEPARATORS
  // --------------------------------------------------------------------------
  lv_obj_t* col_left  = create_box(parent, W_LEFT,  H, false);
  lv_obj_t* sep1      = create_box(parent, 1,       H, false);
  lv_obj_t* col_mid   = create_box(parent, W_MID,   H, false);
  lv_obj_t* sep2      = create_box(parent, 1,       H, false);
  lv_obj_t* col_right = create_box(parent, W_RIGHT, H, false);

  lv_obj_set_style_bg_opa(sep1, LV_OPA_100, 0);
  lv_obj_set_style_bg_color(sep1, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(sep2, LV_OPA_100, 0);
  lv_obj_set_style_bg_color(sep2, lv_color_black(), 0);

  // ==========================================================================
  // LEFT COLUMN — WIND
  // ==========================================================================
  lv_obj_set_style_pad_all(col_left, 14, 0);

  // Compass box
  // Main wind compass area (primary visual element)
  lv_obj_t* compass = create_box(col_left, 180, 180, true);
  lv_obj_align(compass, LV_ALIGN_TOP_LEFT, 0, 10);

  lv_obj_t* lbl_comp = create_label(compass, "NO");
  lv_obj_align(lbl_comp, LV_ALIGN_CENTER, 0, 0);

  // Wind direction + speed
  lv_obj_t* lbl_dir = create_label(col_left, "NO");
  lv_obj_align(lbl_dir, LV_ALIGN_TOP_LEFT, 200, 70);

  lv_obj_t* lbl_ms = create_label(col_left, "5.1 m/s");
  lv_obj_align(lbl_ms,  LV_ALIGN_TOP_LEFT, 200, 115);

  // Divider
  lv_obj_t* line = create_box(col_left, W_LEFT - 28, 1, false);
  lv_obj_set_style_bg_opa(line, LV_OPA_100, 0);
  lv_obj_set_style_bg_color(line, lv_color_black(), 0);
  lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_DIV);

  // Forecast section
  lv_obj_t* lbl_forecast = create_label(col_left, "PROGNOS 1–2 h");
  lv_obj_align(lbl_forecast, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_TITLE);

  lv_obj_t* lbl_future = create_label(col_left,
    "15.3 m/s  ONO\n"
    "11.4 m/s  NNV");
  lv_obj_align(lbl_future, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_CONTENT);

  // ==========================================================================
  // MIDDLE COLUMN — WEATHER + RAIN BARS
  // ==========================================================================
  lv_obj_set_style_pad_all(col_mid, 14, 0);

  // Weather icon placeholder
  lv_obj_t* wx_icon = create_box(col_mid, W_MID - 28, 140, true);
  lv_obj_align(wx_icon, LV_ALIGN_TOP_MID, 0, 10);

  lv_obj_t* lbl_wx = create_label(wx_icon, "[WX ICON]");
  lv_obj_align(lbl_wx, LV_ALIGN_CENTER, 0, 0);

  // Rain title aligned to shared rhythm
  lv_obj_t* lbl_rain = create_label(col_mid, "NEDERBÖRD (risk)");
  lv_obj_align(lbl_rain, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_TITLE);

  // Rain bar plot area
  const lv_coord_t BARS_H = 190;
  lv_obj_t* bars = create_box(col_mid, W_MID - 28, BARS_H, false);
  lv_obj_align(bars, LV_ALIGN_BOTTOM_MID, 0, -14);

  lv_obj_set_flex_flow(bars, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(bars, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
  lv_obj_set_style_pad_gap(bars, 10, 0);

  lv_obj_set_style_border_width(bars, 1, 0);
  lv_obj_set_style_border_color(bars, lv_color_black(), 0);

  const lv_coord_t COL_W = 62;

  for (int i = 0; i < 3; i++) {

    g_rain_col[i] = lv_obj_create(bars);
    lv_obj_clear_flag(g_rain_col[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(g_rain_col[i], COL_W, BARS_H);
    lv_obj_set_style_bg_opa(g_rain_col[i], LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_rain_col[i], 0, 0);

    // Percentage label
    g_rain_pct[i] = create_label(g_rain_col[i], "0%");
    lv_obj_align(g_rain_pct[i], LV_ALIGN_TOP_MID, 0, Y_RAIN_PERCENT);
    lv_obj_move_foreground(g_rain_pct[i]);

    // Bar
    g_rain_bar[i] = lv_obj_create(g_rain_col[i]);
    lv_obj_clear_flag(g_rain_bar[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(g_rain_bar[i], COL_W - 16);
    lv_obj_set_height(g_rain_bar[i], 2);

    lv_obj_set_style_border_width(g_rain_bar[i], 0, 0);
    lv_obj_set_style_bg_opa(g_rain_bar[i], LV_OPA_100, 0);
    lv_obj_set_style_bg_color(g_rain_bar[i], lv_color_black(), 0);

    lv_obj_align(g_rain_bar[i], LV_ALIGN_BOTTOM_MID, 0, Y_RAIN_BAR_BASE_Y);

    // Baseline (1px) to visually anchor the bars
    g_rain_base[i] = lv_obj_create(g_rain_col[i]);
    lv_obj_clear_flag(g_rain_base[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(g_rain_base[i], COL_W - 10, 1);
    lv_obj_set_style_border_width(g_rain_base[i], 0, 0);
    lv_obj_set_style_bg_opa(g_rain_base[i], LV_OPA_100, 0);
    lv_obj_set_style_bg_color(g_rain_base[i], lv_color_black(), 0);

    // Align baseline to the same "ground" as the bars
    lv_obj_align(g_rain_base[i], LV_ALIGN_BOTTOM_MID, 0, Y_RAIN_BAR_BASE_Y);

    lv_obj_move_foreground(g_rain_bar[i]);

    // X-axis label
    lv_obj_t* t = create_label(g_rain_col[i], g_rain_t[i]);
    lv_obj_align(t, LV_ALIGN_BOTTOM_MID, 0, Y_RAIN_XLABEL); // push downward a bit to avoid overlap with baseline
  }

  // ==========================================================================
  // RIGHT COLUMN — TEMPERATURE + ATMOSPHERE
  // ==========================================================================
  lv_obj_set_style_pad_all(col_right, 14, 0);

  // Outdoor
  lv_obj_t* lbl_ute = create_label(col_right, "UTE");
  apply_text_style_title(lbl_ute);
  lv_obj_align(lbl_ute, LV_ALIGN_TOP_LEFT, 0, 0);

  lv_obj_t* ute_val = create_label(col_right, "-22.3 °C");
  apply_text_style_value(ute_val);
  lv_obj_align(ute_val, LV_ALIGN_TOP_LEFT, 0, 22);

  // Indoor
  lv_obj_t* lbl_inne = create_label(col_right, "INNE");
  apply_text_style_title(lbl_inne);
  lv_obj_align(lbl_inne, LV_ALIGN_TOP_LEFT, 0, 110);

  lv_obj_t* inne_val = create_label(col_right, "24.2 °C");
  apply_text_style_value(inne_val);
  lv_obj_align(inne_val, LV_ALIGN_TOP_LEFT, 0, 132);

  // Divider aligned with left column
  lv_obj_t* line2 = create_box(col_right, W_RIGHT - 28, 1, false);
  lv_obj_set_style_bg_opa(line2, LV_OPA_100, 0);
  lv_obj_set_style_bg_color(line2, lv_color_black(), 0);
  lv_obj_align(line2, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_DIV);

  // Atmosphere section (aligned to shared rhythm)
  lv_obj_t* lbl_atm = create_label(col_right, "ATMOSFÄR");
  apply_text_style_title(lbl_atm);
  lv_obj_align(lbl_atm, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_TITLE);

  lv_obj_t* atm_vals = create_label(col_right,
    "Tryck: 1000.3 mbar\n"
    "Fukt:  90 %");
  lv_obj_align(atm_vals, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_CONTENT);

  // Last updated
  lv_obj_t* updated = create_label(col_right, "Uppdaterat: 1 h sedan");
  lv_obj_align(updated, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_style_text_font(updated, &lv_font_montserrat_14, 0);
}


void lvgl_port_begin() {
  // ---- EPaper init ----
  epaper.begin();
  epaper.fillScreen(TFT_WHITE);

  // ---- LVGL init ----
  lv_init();

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


  // ---- page1-only ----
  lv_obj_t *scr = lv_scr_act();
  page1_build(scr);
  page1_update();



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
