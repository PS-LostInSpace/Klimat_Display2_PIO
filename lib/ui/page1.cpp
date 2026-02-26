#include "page1.h"
#include "fonts/kd2_fonts.h"
#include <cstdio>   // snprintf
#include <stdint.h>
#include <math.h>
#include "ui_icons.h"

// -------- Internal UI state --------

// Rain bars
static lv_obj_t* g_rain_bar[3] = {nullptr, nullptr, nullptr};
static lv_obj_t* g_rain_pct[3] = {nullptr, nullptr, nullptr};
static lv_obj_t* g_rain_col[3] = {nullptr, nullptr, nullptr};

static lv_obj_t* g_wx_img = nullptr;

static lv_obj_t* g_lbl_wind_dir = nullptr;
static lv_obj_t* g_lbl_wind_speed = nullptr;
static lv_obj_t* g_wind_arrow = nullptr;
static lv_obj_t* g_compass = nullptr;
static int16_t   g_wind_arrow_deg = 315;

static lv_obj_t* g_lbl_rain_pct[3] = {nullptr,nullptr,nullptr};

static lv_obj_t* g_lbl_temp_out = nullptr;
static lv_obj_t* g_lbl_temp_out_unit = nullptr;

static lv_obj_t* g_lbl_feels = nullptr;
static lv_obj_t* g_lbl_feels_unit = nullptr;

static lv_obj_t* g_lbl_atm_pressure = nullptr; // e.g. "Tryck: 1000.3 mbar"
static lv_obj_t* g_lbl_atm_humidity = nullptr; // e.g. "Fukt:  90 %"
static lv_obj_t* g_lbl_atm_uv       = nullptr; // e.g. "UV:    2.4"
static lv_obj_t* g_lbl_updated      = nullptr; // e.g. "Uppdaterat: 7 min sedan"
static lv_obj_t* g_lbl_forecast_txt = nullptr;

static const char* g_rain_t[3] = {"30", "60", "90"};

static const lv_coord_t Y_RAIN_PERCENT = -8;
static const lv_coord_t Y_RAIN_XLABEL  = 4;
static const lv_coord_t Y_RAIN_BAR_BASE_Y = -20;
static const lv_coord_t PLOT_H = 112;


// -------- Helpers --------

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int rain_bar_index(lv_obj_t* bar)
{
  for(int i = 0; i < 3; i++) if(g_rain_bar[i] == bar) return i;
  return -1;
}

static void set_rain_bar_from_pct(lv_obj_t* bar, uint8_t pct)
{
  if(!bar) return;

  const int idx = rain_bar_index(bar);
  if(idx < 0 || !g_lbl_rain_pct[idx]) return;

  lv_obj_t* col = lv_obj_get_parent(bar);
  if(!col) return;

  // 1) Se till att labelns nya text är layoutad innan vi mäter
  lv_obj_update_layout(col);
  lv_obj_update_layout(g_lbl_rain_pct[idx]);

  // 2) Align barens baseline först så vi vet exakt var bottom hamnar
  lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, Y_RAIN_BAR_BASE_Y);
  lv_obj_update_layout(bar);

  // 3) Mät i absoluta coords (ingen lokal/negativ-y strul)
  lv_area_t bar_a, lbl_a;
  lv_obj_get_coords(bar, &bar_a);
  lv_obj_get_coords(g_lbl_rain_pct[idx], &lbl_a);

  const lv_coord_t base_abs_y = bar_a.y2;      // barens bottom (abs)
  const lv_coord_t top_abs_y  = lbl_a.y2 + 6;  // 6 px under labelns bottom

  lv_coord_t max_h = base_abs_y - top_abs_y;
  if(max_h < 2) max_h = 2;

  int p = clamp_int((int)pct, 0, 100);
  lv_coord_t h = (lv_coord_t)((p * (int)max_h) / 100);
  if(h < 2) h = 2;

  lv_obj_set_height(bar, h);
  lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, Y_RAIN_BAR_BASE_Y);
}


static void format_updated(char* out, size_t out_sz, uint16_t min_ago) {
  if(out_sz == 0) return;

  if(min_ago < 60) {
    snprintf(out, out_sz, "Uppdaterat: %u min sedan", (unsigned)min_ago);
  } else {
    uint16_t h = min_ago / 60;
    uint16_t m = min_ago % 60;
    if(m == 0) snprintf(out, out_sz, "Uppdaterat: %u h sedan", (unsigned)h);
    else       snprintf(out, out_sz, "Uppdaterat: %u h %u min sedan", (unsigned)h, (unsigned)m);
  }
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

static void wind_arrow_event_cb(lv_event_t* e)
{
  if(lv_event_get_code(e) != LV_EVENT_DRAW_MAIN) return;

  lv_obj_t* obj = lv_event_get_target(e);
  lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);
  if(!obj || !draw_ctx) return;

  lv_area_t a;
  lv_obj_get_coords(obj, &a);

  const float cx = (a.x1 + a.x2) * 0.5f;
  const float cy = (a.y1 + a.y2) * 0.5f;

  float r_ring = 88.0f; // fallback
  if(g_compass) {
    lv_area_t ca;
    lv_obj_get_coords(g_compass, &ca);
    const float cs = (float)lv_area_get_width(&ca);
    r_ring = (cs * 0.5f) - 2.0f;
  }

  const float deg2rad = 3.1415926f / 180.0f;
  const float ang = (float)g_wind_arrow_deg * deg2rad;

  const float tri_len = 30.0f;
  const float half_w  = 11.0f;
  const float base_out = 12.0f;

  const float ax = cx + (r_ring + base_out) * sinf(ang);
  const float ay = cy - (r_ring + base_out) * cosf(ang);

  const float tx = cosf(ang);
  const float ty = sinf(ang);
  const float nx = -sinf(ang);
  const float ny =  cosf(ang);

  lv_point_t p0 = {
    (lv_coord_t)(ax + tri_len * nx),
    (lv_coord_t)(ay + tri_len * ny)
  };

  lv_point_t p1 = {
    (lv_coord_t)(ax + half_w * tx),
    (lv_coord_t)(ay + half_w * ty)
  };

  lv_point_t p2 = {
    (lv_coord_t)(ax - half_w * tx),
    (lv_coord_t)(ay - half_w * ty)
  };

  lv_draw_rect_dsc_t tri;
  lv_draw_rect_dsc_init(&tri);
  tri.bg_color = lv_color_black();
  tri.bg_opa   = LV_OPA_COVER;
  tri.border_width = 0;

  lv_point_t tri_pts[3] = { p0, p1, p2 };
  lv_draw_triangle(draw_ctx, &tri, tri_pts);
}


// -------- Page 1 layout --------

void page1_build(lv_obj_t* parent) {

    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_style_pad_gap(parent, 0, 0);

    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(parent, 0, 0);

    const lv_coord_t W_LEFT  = 270;
    const lv_coord_t W_MID   = 260;
    const lv_coord_t W_RIGHT = 270;
    const lv_coord_t H = 480 - 20;

    const lv_coord_t Y_SECTION_DIV = 220;

    const lv_coord_t FEELS_Y_OFFSET = 16;

    const lv_coord_t Y_BELOW_TITLE   = Y_SECTION_DIV + 12;
    const lv_coord_t Y_BELOW_CONTENT = Y_SECTION_DIV + 54;

    const lv_coord_t Y_RAIN_TITLE     = 200;
    const lv_coord_t Y_BARS_BOTTOM    = -6;
    const lv_coord_t BARS_H           = 190;

    const lv_coord_t Y_RAIN_XLABEL_OFFSET = 8;

    lv_obj_t* col_left  = create_box(parent, W_LEFT,  H, false);
    lv_obj_t* sep1      = create_box(parent, 1,       H, false);
    lv_obj_t* col_mid   = create_box(parent, W_MID,   H, false);
    lv_obj_t* sep2      = create_box(parent, 1,       H, false);
    lv_obj_t* col_right = create_box(parent, W_RIGHT, H, false);

    lv_obj_add_flag(col_left, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_clear_flag(col_mid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(col_mid, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    lv_obj_set_style_bg_opa(sep1, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(sep1, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(sep2, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(sep2, lv_color_black(), 0);

    // -------- LEFT: Wind --------
    lv_obj_set_style_pad_all(col_left, 14, 0);

    lv_obj_t* wind_top = create_box(col_left, W_LEFT - 28, Y_SECTION_DIV, false);
    lv_obj_align(wind_top, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_opa(wind_top, LV_OPA_0, 0);
    lv_obj_add_flag(wind_top, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    // Compass "package" container so we can center it vertically in wind_top
    lv_obj_t* compass_wrap = create_box(wind_top, 210, 210, false);
    lv_obj_set_style_bg_opa(compass_wrap, LV_OPA_0, 0);
    lv_obj_set_style_border_width(compass_wrap, 0, 0);
    lv_obj_add_flag(compass_wrap, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    // Raise the whole compass package a bit to better center it in the area
    lv_obj_align(compass_wrap, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t* compass = create_box(compass_wrap, 180, 180, false);
    lv_obj_align(compass, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(compass, LV_OPA_0, 0);
    lv_obj_set_style_border_width(compass, 2, 0);
    lv_obj_set_style_border_color(compass, lv_color_black(), 0);
    lv_obj_set_style_radius(compass, LV_RADIUS_CIRCLE, 0);
    g_compass = compass;

    // Add cardinal directions N, O, S, V around the compass (like N at 0/360)
    lv_obj_t* lbl_n = create_label(compass_wrap, "N");
    ui_set_font(lbl_n, UI_FONT_SMALL);
    lv_obj_align_to(lbl_n, compass, LV_ALIGN_OUT_TOP_MID, 0, -6);

    lv_obj_t* lbl_o = create_label(compass_wrap, "O");
    ui_set_font(lbl_o, UI_FONT_SMALL);
    lv_obj_align_to(lbl_o, compass, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t* lbl_s = create_label(compass_wrap, "S");
    ui_set_font(lbl_s, UI_FONT_SMALL);
    lv_obj_align_to(lbl_s, compass, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);

    lv_obj_t* lbl_v = create_label(compass_wrap, "V");
    ui_set_font(lbl_v, UI_FONT_SMALL);
    lv_obj_align_to(lbl_v, compass, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    // Fylld pil (ritas i draw-event) - skapa sist så den hamnar över labels
    g_wind_arrow = lv_obj_create(compass_wrap);
    lv_obj_set_size(g_wind_arrow, 270, 270);
    lv_obj_center(g_wind_arrow);
    lv_obj_set_style_bg_opa(g_wind_arrow, LV_OPA_0, 0);
    lv_obj_set_style_border_width(g_wind_arrow, 0, 0);
    lv_obj_clear_flag(g_wind_arrow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(g_wind_arrow, wind_arrow_event_cb, LV_EVENT_DRAW_MAIN, nullptr);

    // initläge
    g_wind_arrow_deg = 315;
    lv_obj_invalidate(g_wind_arrow);

    // Triangeln ska alltid ligga över N/O/S/V
    lv_obj_move_foreground(g_wind_arrow);

    // Wind direction in center
    lv_obj_t* lbl_dir = create_label(compass, "VNV");
    ui_set_font(lbl_dir, UI_FONT_H1);
    lv_obj_set_width(lbl_dir, 120);
    lv_obj_set_style_text_align(lbl_dir, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(lbl_dir, LV_ALIGN_CENTER, 0, -24);
    g_lbl_wind_dir = lbl_dir;

    // Speed (manual layout instead of flex to control baseline)
    lv_obj_t* speed_box = create_box(compass, LV_SIZE_CONTENT, LV_SIZE_CONTENT, false);
    lv_obj_set_style_bg_opa(speed_box, LV_OPA_0, 0);
    lv_obj_align(speed_box, LV_ALIGN_CENTER, 0, 26);

    lv_obj_t* lbl_speed = create_label(speed_box, "20.3");
    ui_set_font(lbl_speed, UI_FONT_H1);
    lv_obj_align(lbl_speed, LV_ALIGN_LEFT_MID, 0, 0);
    g_lbl_wind_speed = lbl_speed;

    lv_obj_t* lbl_ms = create_label(speed_box, "m/s");
    ui_set_font(lbl_ms, UI_FONT_BODY);              // du ville ha BODY här
    lv_obj_align_to(lbl_ms, lbl_speed, LV_ALIGN_OUT_RIGHT_BOTTOM, 4, -4);

    lv_obj_t* line = create_box(col_left, W_LEFT - 28, 1, false);
    lv_obj_set_style_bg_opa(line, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(line, lv_color_black(), 0);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_DIV);

    lv_obj_t* lbl_forecast = create_label(col_left, "PROGNOS VIND");
    ui_set_font(lbl_forecast, UI_FONT_SUBTITLE);
    lv_obj_align(lbl_forecast, LV_ALIGN_TOP_LEFT, 0, Y_BELOW_TITLE);

    g_lbl_forecast_txt = create_label(col_left, "");
    ui_set_font(g_lbl_forecast_txt, UI_FONT_BODY);
    lv_label_set_long_mode(g_lbl_forecast_txt, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(g_lbl_forecast_txt, W_LEFT - 28);
    lv_obj_set_height(g_lbl_forecast_txt, 124);
    lv_obj_align(g_lbl_forecast_txt, LV_ALIGN_TOP_LEFT, 0, Y_BELOW_CONTENT);


    // -------- MID: Rain --------
    lv_obj_set_style_pad_all(col_mid, 14, 0);

    lv_obj_t* wx_icon = create_box(col_mid, W_MID - 28, 160, false);
    lv_obj_align(wx_icon, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_add_flag(wx_icon, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    lv_obj_t* img = lv_img_create(wx_icon);
    g_wx_img = img;
    
    // attach to icon system first
    ui_icons_attach_weather_img(g_wx_img);
    
    //then set icon (fallback for now, will be replaced by MQTT data later)
    ui_set_weather_icon("unknown");   // or "rain" as a demo
    lv_obj_center(img);
    
    lv_obj_t* lbl_rain = create_label(col_mid, "NEDERBÖRD (risk)");
    ui_set_font(lbl_rain, UI_FONT_SUBTITLE);
    lv_obj_align(lbl_rain, LV_ALIGN_TOP_LEFT, 0, Y_RAIN_TITLE);

    lv_obj_t* bars = create_box(col_mid, W_MID - 28, BARS_H, false);
    lv_obj_align(bars, LV_ALIGN_BOTTOM_MID, 0, Y_BARS_BOTTOM);

    lv_obj_set_style_border_width(bars, 1, 0);
    lv_obj_set_style_border_color(bars, lv_color_black(), 0);

    lv_obj_set_flex_flow(bars, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bars, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);

    const lv_coord_t COL_W = 62;

    for (int i = 0; i < 3; i++) {
        g_rain_col[i] = lv_obj_create(bars);
        lv_obj_clear_flag(g_rain_col[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(g_rain_col[i], COL_W, BARS_H);

        g_rain_pct[i] = create_label(g_rain_col[i], "0%");
        lv_obj_align(g_rain_pct[i], LV_ALIGN_TOP_MID, 0, Y_RAIN_PERCENT);
        g_lbl_rain_pct[i] = g_rain_pct[i];

        g_rain_bar[i] = lv_obj_create(g_rain_col[i]);
        lv_obj_set_width(g_rain_bar[i], COL_W - 16);
        lv_obj_set_height(g_rain_bar[i], 2);
        lv_obj_set_style_border_width(g_rain_bar[i], 0, 0);
        lv_obj_set_style_bg_opa(g_rain_bar[i], LV_OPA_100, 0);
        lv_obj_set_style_bg_color(g_rain_bar[i], lv_color_black(), 0);
        lv_obj_set_user_data(g_rain_bar[i], (void*)(intptr_t)i);
        lv_obj_align(g_rain_bar[i], LV_ALIGN_BOTTOM_MID, 0, Y_RAIN_BAR_BASE_Y);

        lv_obj_t* t = create_label(g_rain_col[i], g_rain_t[i]);  // "30" "60" "90"
        ui_set_font(t, UI_FONT_BODY);
        lv_obj_set_width(t, COL_W);
        lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(t, LV_LABEL_LONG_CLIP);
        lv_obj_align(t, LV_ALIGN_BOTTOM_MID, 0, Y_RAIN_XLABEL + Y_RAIN_XLABEL_OFFSET);
    }

    // Add "(minuter)" under the rain area with SMALL
    lv_obj_t* lbl_minuter = create_label(col_mid, "(minuter)");
    ui_set_font(lbl_minuter, UI_FONT_SMALL);
    lv_obj_set_style_pad_bottom(lbl_minuter, 2, 0);
    lv_obj_set_style_text_align(lbl_minuter, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lbl_minuter, W_MID - 28);
    lv_obj_align_to(lbl_minuter, bars, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    // -------- RIGHT: Temps & Atmosphere --------
    lv_obj_set_style_pad_all(col_right, 14, 0);

    lv_obj_t* lbl_ute = create_label(col_right, "UTE");
    ui_set_font(lbl_ute, UI_FONT_SUBTITLE);
    lv_obj_align(lbl_ute, LV_ALIGN_TOP_LEFT, 0, 0);

    // Split value and unit so °C can be SMALL and baseline-aligned
    lv_obj_t* ute_val = create_label(col_right, "-22.3");
    ui_set_font(ute_val, UI_FONT_DISPLAY1);
    lv_obj_align(ute_val, LV_ALIGN_TOP_LEFT, 0, 22);

    lv_obj_t* ute_unit = create_label(col_right, "°C");
    ui_set_font(ute_unit, UI_FONT_BODY);
    lv_obj_align_to(ute_unit, ute_val, LV_ALIGN_OUT_RIGHT_BOTTOM, 6, -10);

    g_lbl_temp_out = ute_val;
    g_lbl_temp_out_unit = ute_unit;

    // Replace INNE with KÄNNS SOM
    lv_obj_t* lbl_feels = create_label(col_right, "KÄNNS SOM");
    ui_set_font(lbl_feels, UI_FONT_BODY);
    lv_obj_align(lbl_feels, LV_ALIGN_TOP_LEFT, 0, 110 + FEELS_Y_OFFSET);

    lv_obj_t* feels_val = create_label(col_right, "-23.9");
    ui_set_font(feels_val, UI_FONT_H1);
    lv_obj_align(feels_val, LV_ALIGN_TOP_LEFT, 0, 132 + FEELS_Y_OFFSET);

    lv_obj_t* feels_unit = create_label(col_right, "°C");
    ui_set_font(feels_unit, UI_FONT_BODY);
    lv_obj_align_to(feels_unit, feels_val, LV_ALIGN_OUT_RIGHT_BOTTOM, 6, -4);

    g_lbl_feels = feels_val;
    g_lbl_feels_unit = feels_unit;

    lv_obj_t* line2 = create_box(col_right, W_RIGHT - 28, 1, false);
    lv_obj_set_style_bg_opa(line2, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(line2, lv_color_black(), 0);
    lv_obj_align(line2, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_DIV);

    lv_obj_t* lbl_atm = create_label(col_right, "ATMOSFÄR");
    ui_set_font(lbl_atm, UI_FONT_SUBTITLE);
    lv_obj_align(lbl_atm, LV_ALIGN_TOP_LEFT, 0, Y_BELOW_TITLE);

    // ATMOSFÄR section (replace your atm_vals multiline label)
    g_lbl_atm_pressure = create_label(col_right, "Tryck: --.- mbar");
    lv_obj_align(g_lbl_atm_pressure, LV_ALIGN_TOP_LEFT, 0, Y_BELOW_CONTENT);

    g_lbl_atm_humidity = create_label(col_right, "Fukt:  -- %");
    lv_obj_align_to(g_lbl_atm_humidity, g_lbl_atm_pressure, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

    g_lbl_atm_uv = create_label(col_right, "UV:    --");
    lv_obj_align_to(g_lbl_atm_uv, g_lbl_atm_humidity, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

    // Updated label (store pointer)
    g_lbl_updated = create_label(col_right, "Uppdaterat: --");
    ui_set_font(g_lbl_updated, UI_FONT_SMALL);
    lv_obj_align(g_lbl_updated, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}


// -------- Dynamic update --------

static void set_label_float_1(lv_obj_t* lbl, float v) {
  if(!lbl) return;
  if(isnan(v)) {
    lv_label_set_text(lbl, "--");
    return;
  }
  char buf[16];
  // One decimal
  // NOTE: snprintf is fine here; if you want faster, write your own formatter.
  snprintf(buf, sizeof(buf), "%.1f", v);
  lv_label_set_text(lbl, buf);
}

static void set_label_u16(lv_obj_t* lbl, uint16_t v, const char* suffix) {
  if(!lbl) return;
  char buf[24];
  if(suffix) snprintf(buf, sizeof(buf), "%u%s", (unsigned)v, suffix);
  else       snprintf(buf, sizeof(buf), "%u", (unsigned)v);
  lv_label_set_text(lbl, buf);
}

void page1_update(const ui_state_t* s) {
  if(!s) return;

  // WIND
  if(s->dirty.wind) {
    if(g_lbl_wind_dir) lv_label_set_text(g_lbl_wind_dir, s->wind_dir_txt);
    set_label_float_1(g_lbl_wind_speed, s->wind_ms);

    // Rotate wind arrow (deg)
    if(g_wind_arrow) {
      if(s->wind_deg >= 0) {
        g_wind_arrow_deg = s->wind_deg;
        lv_obj_clear_flag(g_wind_arrow, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(g_wind_arrow);
        lv_obj_invalidate(g_wind_arrow);
      } else {
        lv_obj_add_flag(g_wind_arrow, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }

  // RAIN
  if(s->dirty.rain) {
    for(int i=0;i<3;i++) {
      const uint8_t incoming_pct = s->rain_pct[i];

      if(g_lbl_rain_pct[i]) {
        if(incoming_pct > 100) {
          lv_label_set_text(g_lbl_rain_pct[i], "--");
        } else {
          char buf[8];
          snprintf(buf, sizeof(buf), "%u%%", (unsigned)incoming_pct);
          lv_label_set_text(g_lbl_rain_pct[i], buf);
        }
      }
      if(g_rain_bar[i]) {
        const uint8_t rain_pct = (incoming_pct > 100) ? 0 : incoming_pct;
        set_rain_bar_from_pct(g_rain_bar[i], rain_pct);
      }
    }
  }

  // TEMPS
  if(s->dirty.temps) {
    set_label_float_1(g_lbl_temp_out, s->temp_out_c);
    set_label_float_1(g_lbl_feels, s->feels_like_c);
  }

  // ATM
  if(s->dirty.atmosphere) {
    if(g_lbl_atm_pressure) {
      if(isnan(s->pressure_mbar)) {
        lv_label_set_text(g_lbl_atm_pressure, "Tryck: --.- mbar");
      } else {
        char buf[32];
        snprintf(buf, sizeof(buf), "Tryck: %.1f mbar", s->pressure_mbar);
        lv_label_set_text(g_lbl_atm_pressure, buf);
      }
    }
    if(g_lbl_atm_humidity) {
      if(s->humidity_pct > 100) {
        lv_label_set_text(g_lbl_atm_humidity, "Fukt:  -- %");
      } else {
        char buf[24];
        snprintf(buf, sizeof(buf), "Fukt:  %u %%", (unsigned)s->humidity_pct);
        lv_label_set_text(g_lbl_atm_humidity, buf);
      }
    }
    if(g_lbl_atm_uv) {
      if(isnan(s->uv_index)) {
        lv_label_set_text(g_lbl_atm_uv, "UV:    --");
      } else {
        char buf[24];
        snprintf(buf, sizeof(buf), "UV:    %.1f", s->uv_index);
        lv_label_set_text(g_lbl_atm_uv, buf);
      }
    }
  }

  // FORECAST
  if(s->dirty.forecast) {
    if(g_lbl_forecast_txt) lv_label_set_text(g_lbl_forecast_txt, s->forecast_txt);
  }

  // ICON
  if(s->dirty.wx_icon) {
    ui_set_weather_icon(s->wx_symbol);
  }

  // UPDATED
  if(s->dirty.updated) {
    if(g_lbl_updated) {
      if(s->updated_min_ago == UINT16_MAX) {
        lv_label_set_text(g_lbl_updated, "Uppdaterat: --");
      } else if(s->updated_min_ago == 0) {
        lv_label_set_text(g_lbl_updated, "Uppdaterat: nyss");
      } else {
        char buf[48];
        format_updated(buf, sizeof(buf), s->updated_min_ago);
        lv_label_set_text(g_lbl_updated, buf);
      }
    }
  }
}


