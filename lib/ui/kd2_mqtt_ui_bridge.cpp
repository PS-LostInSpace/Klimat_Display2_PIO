#include "kd2_mqtt_ui_bridge.h"
#include <string.h>
#include <stdlib.h>

// Use ArduinoJson (common on ESP32)
#include <ArduinoJson.h>

static bool copy_str(char* dst, size_t dst_sz, const char* src) {
  if(!dst || dst_sz == 0) return false;
  if(!src) { dst[0] = '\0'; return false; }
  strncpy(dst, src, dst_sz - 1);
  dst[dst_sz - 1] = '\0';
  return true;
}

bool kd2_ui_apply_mqtt_json(ui_state_t* s, const char* json, size_t len) {
  if(!s || !json || len == 0) return false;

  StaticJsonDocument<768> doc; // adjust if needed
  DeserializationError err = deserializeJson(doc, json, len);
  if(err) return false;

  bool changed = false;

  // WIND
  if(doc.containsKey("wind_dir")) {
    const char* v = doc["wind_dir"];
    if(v && strncmp(s->wind_dir_txt, v, sizeof(s->wind_dir_txt)) != 0) {
      copy_str(s->wind_dir_txt, sizeof(s->wind_dir_txt), v);
      s->dirty.wind = true; changed = true;
    }
  }
  if(doc.containsKey("wind_ms")) {
    float v = doc["wind_ms"];
    if(v != s->wind_ms) { s->wind_ms = v; s->dirty.wind = true; changed = true; }
  }
  if(doc.containsKey("wind_deg")) {
    int v = doc["wind_deg"];
    if(v != s->wind_deg) { s->wind_deg = (int16_t)v; s->dirty.wind = true; changed = true; }
  }

  // RAIN
  // Accept either array: "rain_pct":[15,55,35] OR keys: rain_p30/rain_p60/rain_p90
  if(doc.containsKey("rain_pct") && doc["rain_pct"].is<JsonArray>()) {
    JsonArray arr = doc["rain_pct"].as<JsonArray>();
    for(int i=0;i<3;i++) {
      if(i < (int)arr.size()) {
        uint8_t v = (uint8_t)arr[i].as<int>();
        if(v != s->rain_pct[i]) { s->rain_pct[i] = v; s->dirty.rain = true; changed = true; }
      }
    }
  } else {
    const char* keys[3] = {"rain_p30","rain_p60","rain_p90"};
    for(int i=0;i<3;i++) {
      if(doc.containsKey(keys[i])) {
        uint8_t v = (uint8_t)doc[keys[i]].as<int>();
        if(v != s->rain_pct[i]) { s->rain_pct[i] = v; s->dirty.rain = true; changed = true; }
      }
    }
  }

  // TEMPS
  if(doc.containsKey("temp_out")) {
    float v = doc["temp_out"];
    if(v != s->temp_out_c) { s->temp_out_c = v; s->dirty.temps = true; changed = true; }
  }
  if(doc.containsKey("feels_like")) {
    float v = doc["feels_like"];
    if(v != s->feels_like_c) { s->feels_like_c = v; s->dirty.temps = true; changed = true; }
  }

  // ATMOS
  if(doc.containsKey("pressure")) {
    float v = doc["pressure"];
    if(v != s->pressure_mbar) { s->pressure_mbar = v; s->dirty.atmosphere = true; changed = true; }
  }
  if(doc.containsKey("humidity")) {
    uint8_t v = (uint8_t)doc["humidity"].as<int>();
    if(v != s->humidity_pct) { s->humidity_pct = v; s->dirty.atmosphere = true; changed = true; }
  }

  // WX SYMBOL
  if(doc.containsKey("wx_symbol")) {
    const char* v = doc["wx_symbol"];
    if(v && strncmp(s->wx_symbol, v, sizeof(s->wx_symbol)) != 0) {
      copy_str(s->wx_symbol, sizeof(s->wx_symbol), v);
      s->dirty.wx_icon = true; changed = true;
    }
  }

  // UPDATED
  if(doc.containsKey("updated_min")) {
    uint16_t v = (uint16_t)doc["updated_min"].as<int>();
    if(v != s->updated_min_ago) { s->updated_min_ago = v; s->dirty.updated = true; changed = true; }
  }

  if(changed) s->dirty.any = true;
  return changed;
}
