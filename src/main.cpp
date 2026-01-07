#include <Arduino.h>
#include <Wire.h>
#include "config.h"

#ifdef HAS_WIFI
#include "AppNetwork.h"
#endif

#ifdef HAS_WEB_OTA
#include "WebOTA.h"
#endif

#ifdef HAS_LVGL
#include "LvglPort.h"
#endif

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(DEVICE_NAME " starting...");

#ifdef HAS_WIFI
  network_begin();
#endif

#ifdef HAS_WEB_OTA
  // Web OTA requires WiFi up, so only start if connected
  if (network_is_connected()) {
    webota_begin();
  }
#endif

#ifdef HAS_LVGL
  lvgl_port_begin();   // ✅ init LVGL + ePaper (inte loop!)
#endif

  delay(5);
}

void loop() {
#ifdef HAS_WIFI
  network_loop();
#endif

#ifdef HAS_WEB_OTA
  webota_loop();
#endif

#ifdef HAS_LVGL
  lvgl_port_loop();    // ✅ kör LVGL + epaper-update varje varv
#endif

  delay(10);
}
