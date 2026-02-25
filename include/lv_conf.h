#pragma once
#ifndef LV_CONF_H
#define LV_CONF_H //Proof of lv_conf.h is being included
#endif
#define KD2_LVCONF_LOADED 1


// Minimal LVGL config for proof-of-life on ePaper

#define LV_COLOR_DEPTH 8
#define LV_COLOR_16_SWAP 0

#define LV_USE_LOG 0

/* Enable font compression support (required for RLE-compressed fonts) */
#define LV_USE_FONT_COMPRESSED 1

/* If your generated fonts are large (many glyphs / bigger sizes), enable this too */
#define LV_FONT_FMT_TXT_LARGE 1


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

// Built-in fonts - turn these off if you don't use them
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_48 0
