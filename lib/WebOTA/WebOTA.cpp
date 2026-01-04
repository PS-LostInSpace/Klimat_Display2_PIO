#include "WebOTA.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

#include "config.h"
#include "secret_credentials.h" // lives in /include (gitignored)

// --- Web server instance (internal to module) ---
static AsyncWebServer server(80);

// Optional: keep project name consistent with your config
static const char* kProjectTitle = DEVICE_NAME;

static String build_index_html() {
  String ip   = WiFi.isConnected() ? WiFi.localIP().toString() : String("Not connected");
  String rssi = WiFi.isConnected() ? String(WiFi.RSSI()) + " dBm" : String("-");

  String html;
  html += "<!doctype html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>"; html += kProjectTitle; html += "</title>";
  html += "<style>";
  html += "body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;margin:24px;max-width:860px;}";
  html += "h1{margin:0 0 8px 0;font-size:28px;}";
  html += ".card{border:1px solid #ddd;border-radius:14px;padding:16px;margin-top:14px;}";
  html += "table{border-collapse:collapse;width:100%;} td{padding:8px 10px;border-bottom:1px solid #eee;}";
  html += "a.btn{display:inline-block;margin-top:14px;padding:10px 14px;border:1px solid #222;border-radius:10px;text-decoration:none;}";
  html += "a.btn:hover{opacity:.85}";
  html += ".muted{color:#666;font-size:13px;}";
  html += "</style></head><body>";

  html += "<h1>"; html += kProjectTitle; html += "</h1>";
  html += "<div class='card'>";
  html += "<p>Enheten kör och väntar på MQTT-data från Home Assistant.</p>";
  html += "<table>";
  html += "<tr><td><b>IP</b></td><td>"; html += ip; html += "</td></tr>";
  html += "<tr><td><b>RSSI</b></td><td>"; html += rssi; html += "</td></tr>";
  html += "<tr><td><b>Uptime</b></td><td>"; html += String(millis() / 1000); html += " s</td></tr>";
  html += "</table>";

  html += "<a class='btn' href='/update'>OTA / Firmware update</a>";
  html += "<p class='muted'>Tips: /update är lösenordsskyddad.</p>";
  html += "</div>";

  html += "</body></html>";
  return html;
}

void webota_begin() {
#ifdef HAS_WEB_OTA
  // Root page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", build_index_html());
  });

  // Simple health endpoint
  server.on("/health", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // --- Protect OTA portal with Basic Auth ---
  // ElegantOTA supports setAuth(user, pass).
  // Requires both non-empty.
  if (strlen(SECRET_OTA_USER) > 0 && strlen(SECRET_OTA_PASS) > 0) {
    ElegantOTA.setAuth(SECRET_OTA_USER, SECRET_OTA_PASS);
  }

  ElegantOTA.setAutoReboot(true);

  // Async-mode is enabled via build flag:
  // -DELEGANTOTA_USE_ASYNC_WEBSERVER=1
  ElegantOTA.begin(&server);

  server.begin();
#endif
}

void webota_loop() {
#ifdef HAS_WEB_OTA
  // Doesn’t hurt to call; some versions use it for housekeeping.
  ElegantOTA.loop();
#endif
}
