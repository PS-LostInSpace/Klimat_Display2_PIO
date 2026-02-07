#include "ui_state.h"

static ui_state_t g_state;
static bool g_dirty = true; // true at boot to force first render

const ui_state_t* ui_state_get(void) {
  return &g_state;
}

static int16_t clamp_i16(int v, int lo, int hi) {
  if (v < lo) return (int16_t)lo;
  if (v > hi) return (int16_t)hi;
  return (int16_t)v;
}

void ui_state_set_rain(int p30, int p60, int p90) {
  int16_t np30 = clamp_i16(p30, 0, 100);
  int16_t np60 = clamp_i16(p60, 0, 100);
  int16_t np90 = clamp_i16(p90, 0, 100);

  // Only mark dirty if something actually changed
  if (g_state.rain_p30 != np30 || g_state.rain_p60 != np60 || g_state.rain_p90 != np90) {
    g_state.rain_p30 = np30;
    g_state.rain_p60 = np60;
    g_state.rain_p90 = np90;
    g_dirty = true;
  }
}

bool ui_state_is_dirty(void) {
  return g_dirty;
}

void ui_state_clear_dirty(void) {
  g_dirty = false;
}
