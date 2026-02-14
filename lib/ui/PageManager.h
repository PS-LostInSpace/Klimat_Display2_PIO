#pragma once
#include "lvgl.h"
#include "ui_state.h"

#ifdef __cplusplus
extern "C" {
#endif

void pagemgr_begin(lv_obj_t* root, ui_state_t* state);
void pagemgr_update(void);

#ifdef __cplusplus
}
#endif
