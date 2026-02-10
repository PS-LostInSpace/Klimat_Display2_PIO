#pragma once

// Minimal LVGL config for proof-of-life on ePaper

#define LV_COLOR_DEPTH 8
#define LV_COLOR_16_SWAP 0

#define LV_USE_LOG 0

// We use millis() as LVGL tick source
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

// Keep memory usage low-ish
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (64U * 1024U)

// Enable basic widgets we’ll use
#define LV_USE_LABEL 1
#define LV_USE_BTN 1

#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
