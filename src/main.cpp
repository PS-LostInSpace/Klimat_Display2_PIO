#include <Arduino.h>
#include "config.h"

#ifdef HAS_WIFI
#include "AppNetwork.h"
#endif

#ifdef HAS_WEB_OTA
#include "WebOTA.h"
#endif

#ifdef HAS_DISPLAY
#include "Display_reTerminal_E1001.h"
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

#ifdef HAS_DISPLAY
  display_begin();
  display_show_test_screen(); // kan bytas mot status-screen imorgon
#endif
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
