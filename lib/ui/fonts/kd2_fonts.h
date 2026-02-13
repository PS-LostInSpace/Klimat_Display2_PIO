#pragma once

#include "lvgl.h"

// Generated in lib/ui/fonts/font_kd2_epaper_*.c
LV_FONT_DECLARE(font_kd2_epaper_12);
LV_FONT_DECLARE(font_kd2_epaper_16);
LV_FONT_DECLARE(font_kd2_epaper_24);
LV_FONT_DECLARE(font_kd2_epaper_28);
LV_FONT_DECLARE(font_kd2_epaper_40);
LV_FONT_DECLARE(font_kd2_epaper_48);
LV_FONT_DECLARE(font_kd2_epaper_64);
LV_FONT_DECLARE(font_kd2_epaper_80);

// Typography roles (use these in UI code)
#define UI_FONT_TINY      (&font_kd2_epaper_12)
#define UI_FONT_SMALL     (&font_kd2_epaper_16)
#define UI_FONT_BODY      (&font_kd2_epaper_24)
#define UI_FONT_SUBTITLE  (&font_kd2_epaper_28)
#define UI_FONT_H2        (&font_kd2_epaper_40)
#define UI_FONT_H1        (&font_kd2_epaper_48)
#define UI_FONT_DISPLAY2        (&font_kd2_epaper_64)
#define UI_FONT_DISPLAY1        (&font_kd2_epaper_80)

static inline void ui_set_font(lv_obj_t* obj, const lv_font_t* font)
{
    lv_obj_set_style_text_font(obj, font, LV_PART_MAIN);
}
