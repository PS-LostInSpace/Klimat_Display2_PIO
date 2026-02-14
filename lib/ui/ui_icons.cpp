#include "ui_icons.h"
#include <string.h>

// Your generated LVGL image
#include "img/wx_sun_cloud_rain_128.h"

// Keep it simple for now: one dummy + fallback.
// Later: expand mapping (MET/YR symbol_code -> icon pointer).
static lv_obj_t* s_weather_img = NULL;

void ui_icons_attach_weather_img(lv_obj_t* img_obj) {
  s_weather_img = img_obj;
}

static void set_img_src(const void* src) {
  if(!s_weather_img) return;
  lv_img_set_src(s_weather_img, src);
}

void ui_set_weather_icon(const char* symbol) {
  if(!s_weather_img) return;

  // Fallback always works
  if(symbol == NULL || symbol[0] == '\0') {
    set_img_src(&wx_sun_cloud_rain_128);
    return;
  }

  // Dummy mapping examples (extend later)
  // NOTE: keep comparisons cheap; avoid std::string on embedded.
  if(strcmp(symbol, "rain") == 0) {
    set_img_src(&wx_sun_cloud_rain_128);
    return;
  }
  if(strcmp(symbol, "clearsky_day") == 0) {
    set_img_src(&wx_sun_cloud_rain_128); // TODO: replace with a sun icon
    return;
  }

  // Unknown -> fallback
  set_img_src(&wx_sun_cloud_rain_128);
}
