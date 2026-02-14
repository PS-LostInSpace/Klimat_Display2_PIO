#include "kd2_mqtt_ui_bridge.h"
#include "ui_state.h"
#include <ArduinoJson.h>

bool kd2_ui_apply_mqtt_json(ui_state_t* s, const char* json, size_t len)
{
    DynamicJsonDocument doc(512);

    if (deserializeJson(doc, json, len))
        return false;

    // WIND
    if (doc["wind"].is<JsonObject>()) {
    const char* dir = doc["wind"]["dir"] | "";
    float speed = doc["wind"]["speed"] | 0.0f;
    int deg = doc["wind"]["deg"] | -1;

    ui_state_set_wind(s, dir, speed, (int16_t)deg);
    }
    
    // --- Temperature ---
    if (doc["temp"].is<JsonObject>()) {
        ui_state_set_temp(
            s,
            doc["temp"]["out"] | 0.0f,
            doc["temp"]["feels"] | 0.0f
        );
    }

    // --- Rain ---
    if (doc["rain"].is<JsonArray>()) {
        ui_state_set_rain(
            s,
            doc["rain"][0] | 0,
            doc["rain"][1] | 0,
            doc["rain"][2] | 0
        );
    }

    // --- Atmosphere ---
    if (doc["atm"].is<JsonObject>()) {
        ui_state_set_atm(
            s,
            doc["atm"]["pressure"] | 0.0f,
            doc["atm"]["humidity"] | 0
        );
    }

    // --- Updated text ---
    if (doc["updated"].is<const char*>()) {
        ui_state_set_updated(s, doc["updated"]);
    }

    // --- Weather icon ---
    if (doc["icon"].is<const char*>()) {
        ui_state_set_icon(s, doc["icon"]);
    }

    // --- Forecast text ---
    if (doc["forecast"].is<const char*>()) {
        ui_state_set_forecast(s, doc["forecast"]);
    }

    return true;
}
