#pragma once
#include <Arduino.h>

void lvgl_port_begin();
void lvgl_port_loop();

// Anropas när UI ändrats och vi vill trigga ePaper update
bool lvgl_port_needs_epaper_update();
void lvgl_port_clear_epaper_update_flag();
