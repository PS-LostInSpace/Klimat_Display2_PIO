#include "ui_state.h"
#include <string.h>
#include <math.h>

void ui_state_init(ui_state_t* s) {
  if(!s) return;
  memset(s, 0, sizeof(*s));

  // Sensible defaults for "proof-of-life"
  strcpy(s->wind_dir_txt, "--");
  s->wind_ms = NAN;
  s->wind_deg = -1;

  s->rain_pct[0] = 255;
  s->rain_pct[1] = 255;
  s->rain_pct[2] = 255;

  s->temp_out_c = NAN;
  s->feels_like_c = NAN;

  s->pressure_mbar = NAN;
  s->humidity_pct = 255;
  s->uv_index = NAN;

  strcpy(s->wx_symbol, "unknown");
  strcpy(s->forecast_txt, "--\n--");
  s->updated_min_ago = UINT16_MAX;

  ui_state_mark_all_dirty(s);
}

void ui_state_mark_all_dirty(ui_state_t* s) {
  if(!s) return;
  s->dirty.wind = true;
  s->dirty.rain = true;
  s->dirty.temps = true;
  s->dirty.atmosphere = true;
  s->dirty.wx_icon = true;
  s->dirty.forecast = true;
  s->dirty.updated = true;
  s->dirty.any = true;
}

void ui_state_clear_dirty(ui_state_t* s) {
  if(!s) return;
  s->dirty.wind = false;
  s->dirty.rain = false;
  s->dirty.temps = false;
  s->dirty.atmosphere = false;
  s->dirty.wx_icon = false;
  s->dirty.forecast = false;
  s->dirty.updated = false;
  s->dirty.any = false;
}

#include <string.h>  // strncpy, strncmp

static bool str_changed(const char* a, const char* b, size_t maxlen) {
  if(!a) a = "";
  if(!b) b = "";
  return strncmp(a, b, maxlen) != 0;
}

void ui_state_set_wind(ui_state_t* s, const char* dir_txt, float ms, int16_t deg) {
  if(!s) return;

  bool changed = false;

  if(str_changed(s->wind_dir_txt, dir_txt, sizeof(s->wind_dir_txt))) {
    strncpy(s->wind_dir_txt, dir_txt ? dir_txt : "", sizeof(s->wind_dir_txt)-1);
    s->wind_dir_txt[sizeof(s->wind_dir_txt)-1] = '\0';
    changed = true;
  }

  if(s->wind_ms != ms) { s->wind_ms = ms; changed = true; }
  if(s->wind_deg != deg) { s->wind_deg = deg; changed = true; }

  if(changed) { s->dirty.wind = true; s->dirty.any = true; }
}

void ui_state_set_rain(ui_state_t* s, uint8_t p30, uint8_t p60, uint8_t p90) {
  if(!s) return;
  bool changed = false;

  if(s->rain_pct[0] != p30) { s->rain_pct[0] = p30; changed = true; }
  if(s->rain_pct[1] != p60) { s->rain_pct[1] = p60; changed = true; }
  if(s->rain_pct[2] != p90) { s->rain_pct[2] = p90; changed = true; }

  if(changed) { s->dirty.rain = true; s->dirty.any = true; }
}

void ui_state_set_temp(ui_state_t* s, float out_c, float feels_c) {
  if(!s) return;
  bool changed = false;

  if(s->temp_out_c != out_c) { s->temp_out_c = out_c; changed = true; }
  if(s->feels_like_c != feels_c) { s->feels_like_c = feels_c; changed = true; }

  if(changed) { s->dirty.temps = true; s->dirty.any = true; }
}

void ui_state_set_atm(ui_state_t* s, float pressure_mbar, uint8_t humidity_pct, float uv_index) {
  if(!s) return;
  bool changed = false;

  if(s->pressure_mbar != pressure_mbar) { s->pressure_mbar = pressure_mbar; changed = true; }
  if(s->humidity_pct != humidity_pct) { s->humidity_pct = humidity_pct; changed = true; }
  if(s->uv_index != uv_index) { s->uv_index = uv_index; changed = true; }

  if(changed) { s->dirty.atmosphere = true; s->dirty.any = true; }
}

void ui_state_set_updated(ui_state_t* s, uint16_t minutes_ago) {
  if(!s) return;
  if(s->updated_min_ago != minutes_ago) {
    s->updated_min_ago = minutes_ago;
    s->dirty.updated = true;
    s->dirty.any = true;
  }
}

void ui_state_set_icon(ui_state_t* s, const char* symbol) {
  if(!s) return;

  if(str_changed(s->wx_symbol, symbol, sizeof(s->wx_symbol))) {
    strncpy(s->wx_symbol, symbol ? symbol : "", sizeof(s->wx_symbol)-1);
    s->wx_symbol[sizeof(s->wx_symbol)-1] = '\0';
    s->dirty.wx_icon = true;
    s->dirty.any = true;
  }
}

void ui_state_set_forecast(ui_state_t* s, const char* txt) {
  if(!s) return;
  if(!txt) txt = "";
  char normalized[sizeof(s->forecast_txt)];
  size_t w = 0;
  for(size_t i = 0; txt[i] != '\0' && w < sizeof(normalized) - 1; ++i) {
    if(txt[i] == '\r') continue;
    normalized[w++] = txt[i];
  }
  normalized[w] = '\0';

  if(strncmp(s->forecast_txt, normalized, sizeof(s->forecast_txt)) != 0) {
    strncpy(s->forecast_txt, normalized, sizeof(s->forecast_txt)-1);
    s->forecast_txt[sizeof(s->forecast_txt)-1] = '\0';
    s->dirty.forecast = true;
    s->dirty.any = true;
  }
}
