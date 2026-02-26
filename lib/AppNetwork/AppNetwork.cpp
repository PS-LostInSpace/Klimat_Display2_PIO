#include "AppNetwork.h"
#include <WiFi.h>
#include <secret_credentials.h> // lives in /include (gitignored)

static uint32_t s_last_wifi_retry_ms = 0;
static const uint32_t k_wifi_retry_interval_ms = 5000;

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
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  uint32_t now = millis();
  if (now - s_last_wifi_retry_ms < k_wifi_retry_interval_ms) {
    return;
  }

  s_last_wifi_retry_ms = now;
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
}

bool network_is_connected() {
  return WiFi.status() == WL_CONNECTED;
}
