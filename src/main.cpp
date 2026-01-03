#include <Arduino.h>
#include "config.h"
#include "secret_credentials.h"

#ifdef HAS_WIFI
#include "AppNetwork.h"
#endif

#ifdef HAS_OTA
#include "OTA.h"
#endif

#ifdef HAS_DISPLAY
#include "Display_reTerminal_E1001.h"
#endif

void setup() {

#ifdef HAS_WIFI
    network_begin();
#endif

#ifdef HAS_OTA
    ota_begin(SECRET_OTA_HOSTNAME, SECRET_OTA_PASS);
#endif

#ifdef HAS_DISPLAY
  display_begin();
  display_show_test_screen();
#endif
}

void loop() {

#ifdef HAS_WIFI
    network_loop();
#endif

#ifdef HAS_OTA
    ota_loop();
#endif

    delay(10);
}
