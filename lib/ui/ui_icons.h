#pragma once
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Call once after page1_build created the LVGL image object
void ui_icons_attach_weather_img(lv_obj_t* img_obj);

// Set icon based on symbol string (e.g. from HA/MET)
void ui_set_weather_icon(const char* symbol);

#ifdef __cplusplus
}
#endif
