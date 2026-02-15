#include "kd2_mqtt_ui_bridge.h"
#include "ui_state.h"
#include <ArduinoJson.h>

bool kd2_ui_apply_mqtt_json(ui_state_t* s, const char* json, size_t len)
{
    DynamicJsonDocument doc(1024);
    bool applied_any = false;

    DeserializationError err = deserializeJson(doc, json, len);
    if (err) {
        Serial.print("[MQTT->UI] JSON parse failed: ");
        Serial.println(err.c_str());
        return false;
    }

    // WIND (preferred nested format)
    if (doc["wind"].is<JsonObject>()) {
        JsonObject wind = doc["wind"].as<JsonObject>();
        const char* dir = wind["dir"] | "";
        float speed = wind["speed"] | 0.0f;
        int deg = wind["deg"] | -1;
        ui_state_set_wind(s, dir, speed, (int16_t)deg);
        applied_any = true;
    } else if (!doc["wind_dir"].isNull() || !doc["wind_ms"].isNull() || !doc["wind_deg"].isNull()) {
        const char* dir = doc["wind_dir"] | "";
        float speed = doc["wind_ms"] | 0.0f;
        int deg = doc["wind_deg"] | -1;
        ui_state_set_wind(s, dir, speed, (int16_t)deg);
        applied_any = true;
    }

    // Temperature (preferred nested format)
    if (doc["temp"].is<JsonObject>()) {
        JsonObject temp = doc["temp"].as<JsonObject>();
        ui_state_set_temp(s, temp["out"] | 0.0f, temp["feels"] | 0.0f);
        applied_any = true;
    } else if (!doc["temp_out"].isNull() || !doc["temp_feels"].isNull() || !doc["feels_like"].isNull()) {
        ui_state_set_temp(s, doc["temp_out"] | 0.0f, doc["temp_feels"] | (doc["feels_like"] | 0.0f));
        applied_any = true;
    }

    // Rain
    if (doc["rain"].is<JsonArray>()) {
        JsonArray rain = doc["rain"].as<JsonArray>();
        ui_state_set_rain(s, rain[0] | 0, rain[1] | 0, rain[2] | 0);
        applied_any = true;
    } else if (!doc["rain_30"].isNull() || !doc["rain_60"].isNull() || !doc["rain_90"].isNull()) {
        ui_state_set_rain(s, doc["rain_30"] | 0, doc["rain_60"] | 0, doc["rain_90"] | 0);
        applied_any = true;
    }

    // Atmosphere (preferred nested format)
    if (doc["atm"].is<JsonObject>()) {
        JsonObject atm = doc["atm"].as<JsonObject>();
        ui_state_set_atm(s, atm["pressure"] | 0.0f, atm["humidity"] | 0);
        applied_any = true;
    } else if (!doc["pressure"].isNull() || !doc["rh"].isNull() || !doc["humidity"].isNull()) {
        ui_state_set_atm(s, doc["pressure"] | 0.0f, doc["rh"] | (doc["humidity"] | 0));
        applied_any = true;
    }

    // Updated (minutes ago)
    if (!doc["updated_min"].isNull()) {
        ui_state_set_updated(s, doc["updated_min"] | 0);
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
