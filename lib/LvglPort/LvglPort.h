#pragma once
#include <Arduino.h>

void lvgl_port_begin();
void lvgl_port_loop();
void lvgl_port_on_ui_json(const char* payload, size_t len);

// Anropas när UI ändrats och vi vill trigga ePaper update
bool lvgl_port_needs_epaper_update();
void lvgl_port_clear_epaper_update_flag();
