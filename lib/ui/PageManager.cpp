#include "PageManager.h"
#include "page1.h"
#include "page2.h"

// -----------------------------------------------------------------------------
// Internal state
// -----------------------------------------------------------------------------
enum PageId {
  PAGE_1 = 0,
  PAGE_2 = 1,
};

static const PageId DEFAULT_PAGE = PAGE_1;
static PageId g_current_page = DEFAULT_PAGE;

static lv_obj_t*  g_root  = nullptr;
static ui_state_t* g_state = nullptr;

// Optional: enable this if you want PageManager to inject demo values at startup
#ifndef PAGEMGR_DEMO_DATA
#define PAGEMGR_DEMO_DATA 0
#endif

// -----------------------------------------------------------------------------
// Begin
// -----------------------------------------------------------------------------
void pagemgr_begin(lv_obj_t* root, ui_state_t* state) {
  g_root = root;
  g_state = state;

  // Safety: state must exist
  if(!g_state) return;

#if PAGEMGR_DEMO_DATA
  // Demo values (optional). Prefer injecting via lvgl_port_on_ui_json in Step 9.
  strncpy(g_state->wind_dir_txt, "VNV", sizeof(g_state->wind_dir_txt)-1);
  g_state->wind_ms = 20.3f;
  g_state->rain_pct[0] = 15;
  g_state->rain_pct[1] = 55;
  g_state->rain_pct[2] = 35;
  g_state->temp_out_c = -22.3f;
  g_state->feels_like_c = -23.9f;
  strncpy(g_state->wx_symbol, "rain", sizeof(g_state->wx_symbol)-1);
  g_state->updated_min_ago = 7;
  ui_state_mark_all_dirty(g_state);
#endif

  switch (g_current_page) {
    case PAGE_1:
      page1_build(g_root);
      page1_update(g_state);
      ui_state_clear_dirty(g_state);
      break;

    case PAGE_2:
      page2_build(g_root);
      page2_update(g_state);
      ui_state_clear_dirty(g_state);
      break;
  }
}

// -----------------------------------------------------------------------------
// Update (called from LVGL loop)
// -----------------------------------------------------------------------------
void pagemgr_update() {
  if(!g_state) return;
  if(!g_state->dirty.any) return;

  switch (g_current_page) {
    case PAGE_1: page1_update(g_state); break;
    case PAGE_2: page2_update(g_state); break;
  }

  ui_state_clear_dirty(g_state);
}
