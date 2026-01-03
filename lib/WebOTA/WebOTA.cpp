#include "WebOTA.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

static AsyncWebServer server(80);

bool webota_begin(uint16_t port) {
  // Recreate server with port only if you want dynamic ports.
  // Keeping port=80 is simplest; we ignore 'port' parameter for now.
  (void)port;

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain",
      "Klimat_Display2_PIO OK\n"
      "Go to /update for OTA\n");
  });

  AsyncElegantOTA.begin(&server);   // /update endpoint
  server.begin();
  return true;
}

void webota_loop() {
  // Async server, nothing needed here.
}
