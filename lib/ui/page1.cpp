#include "page1.h"
#include "fonts/kd2_fonts.h"
#include <cstdio>   // snprintf
#include <math.h>

// -------- Internal UI state --------

// Rain bars
static lv_obj_t* g_rain_bar[3] = {nullptr, nullptr, nullptr};
static lv_obj_t* g_rain_pct[3] = {nullptr, nullptr, nullptr};
static lv_obj_t* g_rain_col[3] = {nullptr, nullptr, nullptr};

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

static void set_wind_arrow(lv_obj_t* line_head,
                           lv_coord_t size,
                           int16_t deg_from_north) {
    const float deg2rad = 3.1415926f / 180.0f;
    const float a = deg_from_north * deg2rad;
    const float cx = size / 2.0f;
    const float cy = size / 2.0f;
    const float r = (size / 2.0f) - 6.0f;
    const float tip_inset = 5.0f;
    const float base_outset = 16.0f;
    const float head_ang = 7.0f * deg2rad;

    static lv_point_t head_pts[4];
    head_pts[0] = {(lv_coord_t)(cx + (r - tip_inset) * sinf(a)),
                   (lv_coord_t)(cy - (r - tip_inset) * cosf(a))};
    head_pts[1] = {(lv_coord_t)(cx + (r + base_outset) * sinf(a + head_ang)),
                   (lv_coord_t)(cy - (r + base_outset) * cosf(a + head_ang))};
    head_pts[2] = {(lv_coord_t)(cx + (r + base_outset) * sinf(a - head_ang)),
                   (lv_coord_t)(cy - (r + base_outset) * cosf(a - head_ang))};
    head_pts[3] = head_pts[0];
    lv_line_set_points(line_head, head_pts, 4);
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

    const lv_coord_t FEELS_Y_OFFSET = 8;

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

    lv_obj_t* compass = create_box(compass_wrap, 162, 162, false);
    lv_obj_align(compass, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(compass, LV_OPA_0, 0);
    lv_obj_set_style_border_width(compass, 2, 0);
    lv_obj_set_style_border_color(compass, lv_color_black(), 0);
    lv_obj_set_style_radius(compass, LV_RADIUS_CIRCLE, 0);

    lv_obj_t* arrow_head = lv_line_create(compass);
    lv_obj_set_style_line_width(arrow_head, 4, 0);
    lv_obj_set_style_line_color(arrow_head, lv_color_black(), 0);

    set_wind_arrow(arrow_head, 162, 315);

    // Add cardinal directions N, O, S, V around the compass (like N at 0/360)
    lv_obj_t* lbl_n = create_label(compass_wrap, "N");
    ui_set_font(lbl_n, UI_FONT_SMALL);
    lv_obj_align_to(lbl_n, compass, LV_ALIGN_OUT_TOP_MID, 0, -4);

    lv_obj_t* lbl_o = create_label(compass_wrap, "O");
    ui_set_font(lbl_o, UI_FONT_SMALL);
    lv_obj_align_to(lbl_o, compass, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    lv_obj_t* lbl_s = create_label(compass_wrap, "S");
    ui_set_font(lbl_s, UI_FONT_SMALL);
    lv_obj_align_to(lbl_s, compass, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    lv_obj_t* lbl_v = create_label(compass_wrap, "V");
    ui_set_font(lbl_v, UI_FONT_SMALL);
    lv_obj_align_to(lbl_v, compass, LV_ALIGN_OUT_LEFT_MID, -4, 0);

    // Wind direction in center
    lv_obj_t* lbl_dir = create_label(compass, "VNV");
    ui_set_font(lbl_dir, UI_FONT_H2);
    lv_obj_set_width(lbl_dir, 120);
    lv_obj_set_style_text_align(lbl_dir, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(lbl_dir, LV_ALIGN_CENTER, 0, -24);

    // Speed (manual layout instead of flex to control baseline)
    lv_obj_t* speed_box = create_box(compass, LV_SIZE_CONTENT, LV_SIZE_CONTENT, false);
    lv_obj_set_style_bg_opa(speed_box, LV_OPA_0, 0);
    lv_obj_align(speed_box, LV_ALIGN_CENTER, 0, 26);

    lv_obj_t* lbl_speed = create_label(speed_box, "20.3");
    ui_set_font(lbl_speed, UI_FONT_H2);
    lv_obj_align(lbl_speed, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t* lbl_ms = create_label(speed_box, "m/s");
    ui_set_font(lbl_ms, UI_FONT_BODY);              // du ville ha BODY här
    lv_obj_align_to(lbl_ms, lbl_speed, LV_ALIGN_OUT_RIGHT_BOTTOM, 4, -4);

    lv_obj_t* line = create_box(col_left, W_LEFT - 28, 1, false);
    lv_obj_set_style_bg_opa(line, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(line, lv_color_black(), 0);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_DIV);

    lv_obj_t* lbl_forecast = create_label(col_left, "PROGNOS 1-2 h");
    ui_set_font(lbl_forecast, UI_FONT_SUBTITLE);
    lv_obj_align(lbl_forecast, LV_ALIGN_TOP_LEFT, 0, Y_BELOW_TITLE);

    lv_obj_t* lbl_future = create_label(col_left,
        "15.3 m/s  ONO\n"
        "11.4 m/s  NNV");
    lv_obj_align(lbl_future, LV_ALIGN_TOP_LEFT, 0, Y_BELOW_CONTENT);


    // -------- MID: Rain --------
    lv_obj_set_style_pad_all(col_mid, 14, 0);

    lv_obj_t* wx_icon = create_box(col_mid, W_MID - 28, 140, true);
    lv_obj_align(wx_icon, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* lbl_wx = create_label(wx_icon, "[WX ICON]");
    lv_obj_align(lbl_wx, LV_ALIGN_CENTER, 0, 0);

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

        g_rain_bar[i] = lv_obj_create(g_rain_col[i]);
        lv_obj_set_width(g_rain_bar[i], COL_W - 16);
        lv_obj_set_height(g_rain_bar[i], 2);
        lv_obj_set_style_border_width(g_rain_bar[i], 0, 0);
        lv_obj_set_style_bg_opa(g_rain_bar[i], LV_OPA_100, 0);
        lv_obj_set_style_bg_color(g_rain_bar[i], lv_color_black(), 0);
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

    lv_obj_t* line2 = create_box(col_right, W_RIGHT - 28, 1, false);
    lv_obj_set_style_bg_opa(line2, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(line2, lv_color_black(), 0);
    lv_obj_align(line2, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_DIV);

    lv_obj_t* lbl_atm = create_label(col_right, "ATMOSFÄR");
    ui_set_font(lbl_atm, UI_FONT_SUBTITLE);
    lv_obj_align(lbl_atm, LV_ALIGN_TOP_LEFT, 0, Y_BELOW_TITLE);

    lv_obj_t* atm_vals = create_label(col_right,
        "Tryck: 1000.3 mbar\n"
        "Fukt:  90 %");
    lv_obj_align(atm_vals, LV_ALIGN_TOP_LEFT, 0, Y_BELOW_CONTENT);

    lv_obj_t* updated = create_label(col_right, "Uppdaterat: 1 h sedan");
    ui_set_font(updated, UI_FONT_SMALL);
    lv_obj_align(updated, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}



// -------- Dynamic update --------

void page1_update(const ui_state_t* s) {
  if (!s) return;

  const int p[3] = { s->rain_p30, s->rain_p60, s->rain_p90 };

  static int last_p[3] = { -1, -1, -1 };
  if (p[0] == last_p[0] && p[1] == last_p[1] && p[2] == last_p[2]) return;
  last_p[0] = p[0]; last_p[1] = p[1]; last_p[2] = p[2];

  const lv_coord_t PLOT_H = 112;
  const lv_coord_t MIN_H = 2;

  for (int i = 0; i < 3; i++) {
    if (!g_rain_bar[i] || !g_rain_pct[i]) continue;

    lv_coord_t h = (lv_coord_t)((p[i] * PLOT_H) / 100);
    if (h < MIN_H) h = MIN_H;

    lv_obj_set_height(g_rain_bar[i], h);

    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", p[i]);
    lv_label_set_text(g_rain_pct[i], buf);
  }
}


