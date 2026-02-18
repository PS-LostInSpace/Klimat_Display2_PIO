#pragma once
#include <stddef.h>
#include <stdint.h>
#include "ui_state.h"

#ifdef __cplusplus
extern "C" {
#endif

// Call when a JSON payload arrives over MQTT
// Returns true if state changed.
bool kd2_ui_apply_mqtt_json(ui_state_t* s, const char* json, size_t len);

// Timestamp (millis) for the latest successfully parsed MQTT payload.
uint32_t kd2_ui_last_mqtt_rx_ms(void);

#ifdef __cplusplus
}
#endif
