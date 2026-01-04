#include <Arduino.h>
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
  lvgl_port_loop();
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

  delay(10);
}
