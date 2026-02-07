#pragma once
#include <lvgl.h>

// Build Page 1 UI inside given parent object
void page1_build(lv_obj_t* parent);

// Update dynamic values on Page 1 (rain, temps, etc.)
void page1_update();
