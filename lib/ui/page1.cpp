#include "page1.h"
#include <cstdio>   // snprintf

// -------- Internal UI state --------

// Rain bars
static lv_obj_t* g_rain_bar[3] = {nullptr, nullptr, nullptr};
static lv_obj_t* g_rain_pct[3] = {nullptr, nullptr, nullptr};
static lv_obj_t* g_rain_col[3] = {nullptr, nullptr, nullptr};

static const char* g_rain_t[3] = {"30", "60", "90 min"};

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

    const lv_coord_t Y_BELOW_TITLE   = Y_SECTION_DIV + 12;
    const lv_coord_t Y_BELOW_CONTENT = Y_SECTION_DIV + 40;

    lv_obj_t* col_left  = create_box(parent, W_LEFT,  H, false);
    lv_obj_t* sep1      = create_box(parent, 1,       H, false);
    lv_obj_t* col_mid   = create_box(parent, W_MID,   H, false);
    lv_obj_t* sep2      = create_box(parent, 1,       H, false);
    lv_obj_t* col_right = create_box(parent, W_RIGHT, H, false);

    lv_obj_set_style_bg_opa(sep1, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(sep1, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(sep2, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(sep2, lv_color_black(), 0);

    // -------- LEFT: Wind --------
    lv_obj_set_style_pad_all(col_left, 14, 0);

    lv_obj_t* compass = create_box(col_left, 180, 180, true);
    lv_obj_align(compass, LV_ALIGN_TOP_LEFT, 0, 10);

    lv_obj_t* lbl_comp = create_label(compass, "NO");
    lv_obj_align(lbl_comp, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* lbl_dir = create_label(col_left, "NO");
    lv_obj_align(lbl_dir, LV_ALIGN_TOP_LEFT, 200, 70);

    lv_obj_t* lbl_ms = create_label(col_left, "5.1 m/s");
    lv_obj_align(lbl_ms, LV_ALIGN_TOP_LEFT, 200, 115);

    lv_obj_t* line = create_box(col_left, W_LEFT - 28, 1, false);
    lv_obj_set_style_bg_opa(line, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(line, lv_color_black(), 0);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_DIV);

    lv_obj_t* lbl_forecast = create_label(col_left, "PROGNOS 1–2 h");
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

    const lv_coord_t BARS_H = 190;

    lv_obj_t* bars = create_box(col_mid, W_MID - 28, BARS_H, false);
    lv_obj_align(bars, LV_ALIGN_BOTTOM_MID, 0, -14);

    lv_obj_set_style_border_width(bars, 1, 0);
    lv_obj_set_style_border_color(bars, lv_color_black(), 0);

    lv_obj_t* lbl_rain = create_label(col_mid, "NEDERBÖRD (risk)");
    lv_obj_align(lbl_rain, LV_ALIGN_TOP_LEFT, 0, 200);

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

        lv_obj_t* t = create_label(g_rain_col[i], g_rain_t[i]);
        lv_obj_align(t, LV_ALIGN_BOTTOM_MID, 0, Y_RAIN_XLABEL);
    }


    // -------- RIGHT: Temps & Atmosphere --------
    lv_obj_set_style_pad_all(col_right, 14, 0);

    lv_obj_t* lbl_ute = create_label(col_right, "UTE");
    lv_obj_align(lbl_ute, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t* ute_val = create_label(col_right, "-22.3 °C");
    lv_obj_align(ute_val, LV_ALIGN_TOP_LEFT, 0, 22);

    lv_obj_t* lbl_inne = create_label(col_right, "INNE");
    lv_obj_align(lbl_inne, LV_ALIGN_TOP_LEFT, 0, 110);

    lv_obj_t* inne_val = create_label(col_right, "24.2 °C");
    lv_obj_align(inne_val, LV_ALIGN_TOP_LEFT, 0, 132);

    lv_obj_t* line2 = create_box(col_right, W_RIGHT - 28, 1, false);
    lv_obj_set_style_bg_opa(line2, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(line2, lv_color_black(), 0);
    lv_obj_align(line2, LV_ALIGN_TOP_LEFT, 0, Y_SECTION_DIV);

    lv_obj_t* lbl_atm = create_label(col_right, "ATMOSFÄR");
    lv_obj_align(lbl_atm, LV_ALIGN_TOP_LEFT, 0, Y_BELOW_TITLE);

    lv_obj_t* atm_vals = create_label(col_right,
        "Tryck: 1000.3 mbar\n"
        "Fukt:  90 %");
    lv_obj_align(atm_vals, LV_ALIGN_TOP_LEFT, 0,  Y_BELOW_CONTENT);

    lv_obj_t* updated = create_label(col_right, "Uppdaterat: 1 h sedan");
    lv_obj_align(updated, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}


// -------- Dynamic update --------

void page1_update() {

    const int p[3] = {15, 55, 35};
    const lv_coord_t PLOT_H = PLOT_H;

    for (int i = 0; i < 3; i++) {
        if (!g_rain_bar[i]) continue;

        lv_coord_t h = (p[i] * PLOT_H) / 100;
        if (h < 2) h = 2;

        lv_obj_set_height(g_rain_bar[i], h);

        char buf[8];
        snprintf(buf, sizeof(buf), "%d%%", p[i]);
        lv_label_set_text(g_rain_pct[i], buf);
    }
}
