#include <Arduino.h>   // millis()
#include "PageManager.h"
#include "page1.h"
#include "ui_state.h"


// -----------------------------------------------------------------------------
// DEBUG: periodically change rain values to verify state → UI → E-Ink refresh
// -----------------------------------------------------------------------------
static void debug_rain_cycle() {
  static uint32_t last_ms = 0;
  static int step = 0;

  uint32_t now = millis();
  if (now - last_ms < 10000) return; // every 10 seconds

  last_ms = now;

  switch (step) {
    case 0: ui_state_set_rain(10, 40, 70); break;
    case 1: ui_state_set_rain(80, 20, 50); break;
    case 2: ui_state_set_rain(0,  0,  0);  break;
    case 3: ui_state_set_rain(100, 60, 30); break;
  }

  step = (step + 1) % 4;
}


// -----------------------------------------------------------------------------
// Internal state
// -----------------------------------------------------------------------------

enum PageId {
    PAGE_1 = 0,
};

static PageId g_current_page = PAGE_1;
static lv_obj_t* g_root = nullptr;


// -----------------------------------------------------------------------------
// Begin
// -----------------------------------------------------------------------------

void pagemgr_begin(lv_obj_t* root) {
  g_root = root;

  // Dummy data for now (later: MQTT/API)
  ui_state_set_rain(15, 55, 35);

  switch (g_current_page) {
    case PAGE_1:
      page1_build(g_root);
      // Force first update (dirty at boot anyway)
      page1_update(ui_state_get());
      ui_state_clear_dirty();
      break;
  }
}



// -----------------------------------------------------------------------------
// Update (called from LVGL loop)
// -----------------------------------------------------------------------------

void pagemgr_update() {

  // DEBUG generator (temporary for Step 8C verification)
  debug_rain_cycle();

  // Only update UI when state changed
  if (!ui_state_is_dirty()) return;

  switch (g_current_page) {
    case PAGE_1:
      page1_update(ui_state_get());
      break;
  }

  ui_state_clear_dirty();
}


