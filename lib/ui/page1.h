#pragma once
#include <lvgl.h>
#include "ui_state.h"

// Build Page 1 UI inside given parent object
void page1_build(lv_obj_t* parent);

// Update dynamic values on Page 1 based on state
void page1_update(const ui_state_t* s);
