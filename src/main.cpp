#include <Arduino.h>
#include "config.h"

#ifdef HAS_DISPLAY
  #include "Display_reTerminal_E1001.h"
#endif

#ifdef HAS_WIFI
  #include "Network.h"
#endif

#ifdef HAS_OTA
  #include "OTA.h"
#endif

void setup() {
    Serial.begin(115200);
    delay(1000);
    while (!Serial) {
        delay(10);
    }
    Serial.println(DEVICE_NAME " booting...");

#ifdef HAS_DISPLAY
  display_init();
#endif

#ifdef HAS_WIFI
  network_init();
#endif

#ifdef HAS_OTA
  ota_init();
#endif
}

void loop() {
#ifdef HAS_DISPLAY
  display_update();
#endif

#ifdef HAS_WIFI
  network_loop();
#endif

#ifdef HAS_OTA
  ota_loop();
#endif

  delay(10);
}
