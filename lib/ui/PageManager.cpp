#include "PageManager.h"
#include "page1.h"
#include "ui_state.h"

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
  if (!ui_state_is_dirty()) return;

  switch (g_current_page) {
    case PAGE_1:
      page1_update(ui_state_get());
      break;
  }

  ui_state_clear_dirty();
}

