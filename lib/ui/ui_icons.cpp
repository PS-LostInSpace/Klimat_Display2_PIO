#include "ui_icons.h"
#include <string.h>
#include <ctype.h>

#include "img/ui_weather_assets.h"

static lv_obj_t* s_weather_img = NULL;

void ui_icons_attach_weather_img(lv_obj_t* img_obj) {
  s_weather_img = img_obj;
  if(s_weather_img) {
    lv_obj_clear_flag(s_weather_img, LV_OBJ_FLAG_HIDDEN);
    lv_img_set_src(s_weather_img, &kd_weather_unknown_192);
  }
}

static bool icontains(const char* haystack, const char* needle) {
  if(!haystack || !needle) return false;

  const size_t nlen = strlen(needle);
  for(const char* p = haystack; *p; p++) {
    size_t i = 0;
    while(p[i] && i < nlen) {
      const char a = (char)tolower((unsigned char)p[i]);
      const char b = (char)tolower((unsigned char)needle[i]);
      if(a != b) break;
      i++;
    }
    if(i == nlen) return true;
  }
  return false;
}

static const lv_img_dsc_t* map_symbol_to_icon_192(const char* s) {
  if(!s || !*s) return &kd_weather_unknown_192;

  if(icontains(s, "unknown") || icontains(s, "null") || icontains(s, "none")) {
    return &kd_weather_unknown_192;
  }

  if(icontains(s, "clear-night") || icontains(s, "_night") || icontains(s, "night")) {
    return &kd_weather_clear_night_192;
  }

  if(icontains(s, "fog")) return &kd_weather_fog_192;
  if(icontains(s, "wind")) return &kd_weather_wind_192;

  if(icontains(s, "sleet") || icontains(s, "snowy-rainy")) return &kd_weather_sleet_192;
  if(icontains(s, "snow")) return &kd_weather_snow_192;

  if(icontains(s, "rain") || icontains(s, "pour") || icontains(s, "showers") || icontains(s, "hail") || icontains(s, "lightning-rainy")) {
    return &kd_weather_rain_192;
  }

  if(icontains(s, "partly") || icontains(s, "fair")) return &kd_weather_partly_192;
  if(icontains(s, "cloud")) return &kd_weather_cloud_192;

  if(icontains(s, "clearsky") || icontains(s, "sunny") || icontains(s, "clear-day")) {
    return &kd_weather_sun_192;
  }

  return &kd_weather_unknown_192;
}

void ui_set_weather_icon(const char* symbol) {
  if(!s_weather_img) return;

  const lv_img_dsc_t* img = map_symbol_to_icon_192(symbol);
  lv_img_set_src(s_weather_img, img);
}
