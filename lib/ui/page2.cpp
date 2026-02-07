#include "page2.h"

// Simple stub: show a centered label
static lv_obj_t* g_label = nullptr;

void page2_build(lv_obj_t* parent) {
  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

  g_label = lv_label_create(parent);
  lv_label_set_text(g_label, "PAGE 2 (stub)");
  lv_obj_align(g_label, LV_ALIGN_CENTER, 0, 0);
}

void page2_update(const ui_state_t* s) {
  (void)s;
  // No dynamic content yet
}
