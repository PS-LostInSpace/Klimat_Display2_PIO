#include "ui_state.h"
#include <string.h>

void ui_state_init(ui_state_t* s) {
  if(!s) return;
  memset(s, 0, sizeof(*s));

  // Sensible defaults for "proof-of-life"
  strcpy(s->wind_dir_txt, "N");
  s->wind_ms = 0.0f;
  s->wind_deg = -1;

  s->rain_pct[0] = 0;
  s->rain_pct[1] = 0;
  s->rain_pct[2] = 0;

  s->temp_out_c = 0.0f;
  s->feels_like_c = 0.0f;

  s->pressure_mbar = 1013.2f;
  s->humidity_pct = 50;

  strcpy(s->wx_symbol, "unknown");
  s->updated_min_ago = 0;

  ui_state_mark_all_dirty(s);
}

void ui_state_mark_all_dirty(ui_state_t* s) {
  if(!s) return;
  s->dirty.wind = true;
  s->dirty.rain = true;
  s->dirty.temps = true;
  s->dirty.atmosphere = true;
  s->dirty.wx_icon = true;
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
  s->dirty.updated = false;
  s->dirty.any = false;
}