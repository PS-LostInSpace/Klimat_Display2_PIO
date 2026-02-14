#include "Kd2Mqtt.h"
#include <WiFi.h>
#define MQTT_MAX_PACKET_SIZE 1024
#include <PubSubClient.h>
#include "secret_credentials.h"   // SECRET_SSID, SECRET_PASS, MQTT creds
#include "LvglPort.h"
#include <cstring>

#if defined(SECRET_WIFI_SSID)
#define KD2_WIFI_SSID SECRET_WIFI_SSID
#elif defined(SECRET_SSID)
#define KD2_WIFI_SSID SECRET_SSID
#endif

#if defined(SECRET_WIFI_PASS)
#define KD2_WIFI_PASS SECRET_WIFI_PASS
#elif defined(SECRET_PASS)
#define KD2_WIFI_PASS SECRET_PASS
#endif

#if defined(SECRET_MQTT_USERNAME2)
#define KD2_MQTT_USER SECRET_MQTT_USERNAME2
#elif defined(SECRET_MQTT_USERNAME)
#define KD2_MQTT_USER SECRET_MQTT_USERNAME
#endif

#if defined(SECRET_MQTT_PASS2)
#define KD2_MQTT_PASS SECRET_MQTT_PASS2
#elif defined(SECRET_MQTT_PASS)
#define KD2_MQTT_PASS SECRET_MQTT_PASS
#endif

#if defined(SECRET_MQTT_HOST)
#define KD2_MQTT_HOST SECRET_MQTT_HOST
#elif defined(SECRET_TPi_IP)
#define KD2_MQTT_HOST SECRET_TPi_IP
#else
#define KD2_MQTT_HOST "127.0.0.1"
#endif

#if defined(SECRET_MQTT_PORT)
#define KD2_MQTT_PORT SECRET_MQTT_PORT
#else
#define KD2_MQTT_PORT 1883
#endif

// ---- MQTT config ----
static const char* kTopicPage1 = "kd2/page1/state";

static WiFiClient espClient;
static PubSubClient client(espClient);

static void mqtt_callback(char* topic, byte* payload, unsigned int length);

// Basic WiFi connect (keep it minimal for now)
static void ensure_wifi() {
  if(WiFi.status() == WL_CONNECTED) return;
  WiFi.begin(KD2_WIFI_SSID, KD2_WIFI_PASS);
  uint32_t t0 = millis();
  while(WiFi.status() != WL_CONNECTED && (millis() - t0) < 15000) {
    delay(250);
  }
}

bool kd2_mqtt_is_connected() {
  return client.connected();
}

static void reconnect_mqtt() {
  static uint32_t lastAttempt = 0;
  static uint32_t intervalMs = 5000;
  static uint8_t fails = 0;

  if(millis() - lastAttempt < intervalMs) return;
  lastAttempt = millis();

  bool connected = false;

#if defined(KD2_MQTT_USER) && defined(KD2_MQTT_PASS)
  connected = client.connect("KD2_ePaper", KD2_MQTT_USER, KD2_MQTT_PASS);
#else
  connected = client.connect("KD2_ePaper");
#endif

  if(connected) {

    intervalMs = 5000;
    fails = 0;

    client.subscribe(kTopicPage1);
    return;
  }

  // backoff (max ~5min)
  fails++;
  intervalMs = min<uint32_t>(intervalMs * 2, 300000);
  if(fails >= 12) ESP.restart();
}

void kd2_mqtt_begin() {
  ensure_wifi();
  client.setServer(KD2_MQTT_HOST, KD2_MQTT_PORT);
  client.setCallback(mqtt_callback);
}

void kd2_mqtt_loop() {
  ensure_wifi();
  if(!client.connected()) reconnect_mqtt();
  client.loop();
}

static void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic, kTopicPage1) != 0) return;
  lvgl_port_on_ui_json((const char*)payload, (size_t)length);
}
