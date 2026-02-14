#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Keep this small and stable so it can be reused across pages and projects.
typedef struct {
  // Wind
  char   wind_dir_txt[8];   // e.g. "VNV"
  float  wind_ms;           // e.g. 2.3
  int16_t wind_deg;         // optional; -1 if unknown

  // Rain (risk % for 30/60/90)
  uint8_t rain_pct[3];      // 0..100

  // Temperatures
  float temp_out_c;         // e.g. -22.3
  float feels_like_c;       // e.g. -23.9

  // Atmosphere
  float pressure_mbar;      // e.g. 1000.3
  uint8_t humidity_pct;     // e.g. 90

  // Weather symbol id (from HA/MET/etc)
  char wx_symbol[32];       // e.g. "clearsky_day" or "rain"

  // UX
  uint16_t updated_min_ago; // e.g. 7

  // Dirty flags to avoid unnecessary redraws/refresh
  struct {
    bool wind;
    bool rain;
    bool temps;
    bool atmosphere;
    bool wx_icon;
    bool updated;
    bool any;
  } dirty;

} ui_state_t;

void ui_state_init(ui_state_t* s);
void ui_state_mark_all_dirty(ui_state_t* s);
void ui_state_clear_dirty(ui_state_t* s);

#ifdef __cplusplus
}
#endif