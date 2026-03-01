#include "ui_icon_test.h"

LV_IMG_DECLARE(kd_weather_partly_128);
LV_IMG_DECLARE(kd_weather_partly_169);
LV_IMG_DECLARE(kd_weather_partly_192);

static void add_icon_column(lv_obj_t* parent, const lv_img_dsc_t* icon, const char* label) {
  lv_obj_t* col = lv_obj_create(parent);
  lv_obj_set_size(col, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(col, LV_OPA_0, 0);
  lv_obj_set_style_border_width(col, 0, 0);
  lv_obj_set_style_pad_all(col, 0, 0);
  lv_obj_set_style_pad_gap(col, 10, 0);
  lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t* img = lv_img_create(col);
  lv_img_set_src(img, icon);

  lv_obj_t* txt = lv_label_create(col);
  lv_label_set_text(txt, label);
  lv_obj_set_style_text_color(txt, lv_color_hex(0xAAAAAA), 0);
}

void ui_icon_test_build(lv_obj_t* parent) {
  if(!parent) return;

  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(parent, 0, 0);
  lv_obj_set_style_bg_color(parent, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(parent, 0, 0);
  lv_obj_set_style_pad_all(parent, 0, 0);
  lv_obj_set_style_pad_gap(parent, 0, 0);
  lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t* title = lv_label_create(parent);
  lv_label_set_text(title, "UI ICON TEST 128/169/192");
  lv_obj_set_style_text_color(title, lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_pad_top(title, 10, 0);

  lv_obj_t* swatch = lv_obj_create(parent);
  lv_obj_set_size(swatch, 260, 38);
  lv_obj_set_style_bg_color(swatch, lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_bg_opa(swatch, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(swatch, lv_color_black(), 0);
  lv_obj_set_style_border_width(swatch, 1, 0);
  lv_obj_set_style_radius(swatch, 0, 0);
  lv_obj_set_style_pad_all(swatch, 0, 0);
  lv_obj_clear_flag(swatch, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t* swatch_txt = lv_label_create(swatch);
  lv_label_set_text(swatch_txt, "LIGHT GRAY TEST #AAAAAA");
  lv_obj_set_style_text_color(swatch_txt, lv_color_black(), 0);
  lv_obj_center(swatch_txt);

  lv_obj_t* row = lv_obj_create(parent);
  lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(row, LV_OPA_0, 0);
  lv_obj_set_style_border_width(row, 0, 0);
  lv_obj_set_style_pad_all(row, 0, 0);
  lv_obj_set_style_pad_gap(row, 24, 0);
  lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_top(row, 16, 0);

  add_icon_column(row, &kd_weather_partly_128, "128");
  add_icon_column(row, &kd_weather_partly_169, "169");
  add_icon_column(row, &kd_weather_partly_192, "192");
}
