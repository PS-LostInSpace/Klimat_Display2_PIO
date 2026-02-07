#include "PageManager.h"
#include "page1.h"
#include "page2.h"
#include "ui_state.h"

// -----------------------------------------------------------------------------
// Internal state
// -----------------------------------------------------------------------------

enum PageId {
  PAGE_1 = 0,
  PAGE_2 = 1,
};


static const PageId DEFAULT_PAGE = PAGE_1;  // keep Page 1 as default
static PageId g_current_page = DEFAULT_PAGE;

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
      page1_update(ui_state_get());
      break;

    case PAGE_2:
      page2_build(g_root);
      page2_update(ui_state_get());
      break;
  }
}



// -----------------------------------------------------------------------------
// Update (called from LVGL loop)
// -----------------------------------------------------------------------------

void pagemgr_update() {
  if (!ui_state_is_dirty()) return;

  switch (g_current_page) {
    case PAGE_1: page1_update(ui_state_get()); break;
    case PAGE_2: page2_update(ui_state_get()); break;
  }

  ui_state_clear_dirty();
}


