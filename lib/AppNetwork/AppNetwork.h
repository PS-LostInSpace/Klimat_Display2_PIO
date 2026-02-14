#pragma once

bool network_begin();
void network_loop();
bool network_is_connected();

void kd2_mqtt_begin();
void kd2_mqtt_loop();
bool kd2_mqtt_is_connected();
