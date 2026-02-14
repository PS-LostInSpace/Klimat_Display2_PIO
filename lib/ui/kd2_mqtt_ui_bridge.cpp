#include "kd2_mqtt_ui_bridge.h"
#include "ui_state.h"
#include <ArduinoJson.h>

bool kd2_ui_apply_mqtt_json(ui_state_t* s, const char* json, size_t len)
{
    DynamicJsonDocument doc(512);
    bool applied_any = false;

    DeserializationError err = deserializeJson(doc, json, len);
    if (err) {
        Serial.print("[MQTT->UI] JSON parse failed: ");
        Serial.println(err.c_str());
        return false;
    }

    // WIND
    if (doc["wind"].is<JsonObject>()) {
    const char* dir = doc["wind"]["dir"] | "";
    float speed = doc["wind"]["speed"] | 0.0f;
    int deg = doc["wind"]["deg"] | -1;

    ui_state_set_wind(s, dir, speed, (int16_t)deg);
    applied_any = true;
    }
    else if (!doc["wind_dir"].isNull() || !doc["wind_ms"].isNull() || !doc["wind_deg"].isNull()) {
        const char* dir = doc["wind_dir"] | "";
        float speed = doc["wind_ms"] | 0.0f;
        int deg = doc["wind_deg"] | -1;

        ui_state_set_wind(s, dir, speed, (int16_t)deg);
        applied_any = true;
    }
    
    // --- Temperature ---
    if (doc["temp"].is<JsonObject>()) {
        ui_state_set_temp(
            s,
            doc["temp"]["out"] | 0.0f,
            doc["temp"]["feels"] | 0.0f
        );
        applied_any = true;
    }
    else if (!doc["temp_out"].isNull() || !doc["temp_feels"].isNull() || !doc["feels_like"].isNull()) {
        ui_state_set_temp(
            s,
            doc["temp_out"] | 0.0f,
            doc["temp_feels"] | (doc["feels_like"] | 0.0f)
        );
        applied_any = true;
    }

    // --- Rain ---
    if (doc["rain"].is<JsonArray>()) {
        ui_state_set_rain(
            s,
            doc["rain"][0] | 0,
            doc["rain"][1] | 0,
            doc["rain"][2] | 0
        );
        applied_any = true;
    }
    else if (!doc["rain_30"].isNull() || !doc["rain_60"].isNull() || !doc["rain_90"].isNull()) {
        ui_state_set_rain(
            s,
            doc["rain_30"] | 0,
            doc["rain_60"] | 0,
            doc["rain_90"] | 0
        );
        applied_any = true;
    }

    // --- Atmosphere ---
    if (doc["atm"].is<JsonObject>()) {
        ui_state_set_atm(
            s,
            doc["atm"]["pressure"] | 0.0f,
            doc["atm"]["humidity"] | 0
        );
        applied_any = true;
    }
    else if (!doc["pressure"].isNull() || !doc["rh"].isNull() || !doc["humidity"].isNull()) {
        ui_state_set_atm(
            s,
            doc["pressure"] | 0.0f,
            doc["rh"] | (doc["humidity"] | 0)
        );
        applied_any = true;
    }

    // --- Updated (minutes ago) ---
    if (doc["updated_min"].is<uint16_t>()) {
        ui_state_set_updated(s, doc["updated_min"]);
        applied_any = true;
    }

    // --- Weather icon ---
    if (doc["icon"].is<const char*>()) {
        ui_state_set_icon(s, doc["icon"]);
        applied_any = true;
    }

    // --- Forecast text ---
    if (doc["forecast"].is<const char*>()) {
        ui_state_set_forecast(s, doc["forecast"]);
        applied_any = true;
    }

    if (!applied_any) {
        Serial.println("[JSON] parsed but no known keys found");
    }

    return true;
}
