#pragma once
#include <stdint.h>
#include <stdbool.h>

// Central UI state container (future: filled by MQTT/API)
typedef struct {
  // Rain probability (%)
  int16_t rain_p30;
  int16_t rain_p60;
  int16_t rain_p90;

  // Wind (future)
  // int16_t wind_deg;
  // float wind_ms;

  // Temps (future)
  // float temp_out;
  // float temp_in;

  // Meta
  uint32_t updated_ms;
} ui_state_t;

// Read-only access to current state
const ui_state_t* ui_state_get(void);

// Mark state as changed + set values
void ui_state_set_rain(int p30, int p60, int p90);

// Dirty flag API
bool ui_state_is_dirty(void);
void ui_state_clear_dirty(void);
