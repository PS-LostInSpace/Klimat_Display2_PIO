#include "AppNetwork.h"
#include <WiFi.h>
#include <secret_credentials.h> // lives in /include (gitignored)

bool network_begin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    if (millis() - start > 15000) return false;
  }
  return true;
}

void network_loop() {
    // reserverad för framtida behov
}

bool network_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}
