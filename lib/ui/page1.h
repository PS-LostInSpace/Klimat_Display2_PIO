#pragma once
#include "lvgl.h"
#include "ui_state.h"

#ifdef __cplusplus
extern "C" {
#endif

void page1_build(lv_obj_t* parent);
void page1_update(const ui_state_t* s);

#ifdef __cplusplus
}
#endif
