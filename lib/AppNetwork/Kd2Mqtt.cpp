#include "Kd2Mqtt.h"
#include <WiFi.h>
#include <Arduino.h>
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
static const char* kTopicPage1 = "home/display/kd2/page1/state";

static WiFiClient espClient;
static PubSubClient client(espClient);
static char g_mqtt_client_id[32] = {0};

static void mqtt_callback(char* topic, byte* payload, unsigned int length);

static void build_client_id() {
  uint64_t chip_id = ESP.getEfuseMac();
  uint16_t hi = (uint16_t)(chip_id >> 32);
  uint32_t lo = (uint32_t)chip_id;
  snprintf(g_mqtt_client_id, sizeof(g_mqtt_client_id), "KD2_%04X%08X", hi, lo);
}

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

  if (WiFi.status() != WL_CONNECTED) return;

  if(millis() - lastAttempt < intervalMs) return;
  lastAttempt = millis();

  Serial.printf("[MQTT] reconnect attempt host=%s port=%u\n", KD2_MQTT_HOST, (unsigned)KD2_MQTT_PORT);

  if(client.connect(g_mqtt_client_id, KD2_MQTT_USER, KD2_MQTT_PASS)) {

    intervalMs = 5000;
    fails = 0;

    Serial.println("[MQTT] connected");

    bool sub_ok = client.subscribe(kTopicPage1);
    Serial.printf("[MQTT] subscribed: %s (%s)\n", kTopicPage1, sub_ok ? "ok" : "failed");
    return;
  }

  Serial.printf("[MQTT] connect failed, state=%d\n", client.state());

  // backoff (max ~5min)
  fails++;
  intervalMs = min<uint32_t>(intervalMs * 2, 300000);
  if(fails >= 12) ESP.restart();
}

void kd2_mqtt_begin() {
  ensure_wifi();
  build_client_id();
  const uint16_t mqtt_buf_size = 1024;
  bool buf_ok = client.setBufferSize(mqtt_buf_size);
  Serial.printf("[MQTT] setBufferSize(%u): %s\n", (unsigned)mqtt_buf_size, buf_ok ? "ok" : "failed");
  client.setKeepAlive(120);
  client.setSocketTimeout(15);
  Serial.printf("[MQTT] clientId=%s\n", g_mqtt_client_id);
#if defined(SECRET_MQTT_HOST) && defined(SECRET_MQTT_PORT)
  client.setServer(SECRET_MQTT_HOST, SECRET_MQTT_PORT);
#else
  client.setServer(KD2_MQTT_HOST, KD2_MQTT_PORT);
#endif
  client.setCallback(mqtt_callback);
}

void kd2_mqtt_loop() {
  ensure_wifi();
  if(!client.connected()) {
    reconnect_mqtt();
    return;
  }
  client.loop();
}

static void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("[MQTT] callback TRIGGERED");

  Serial.print("[MQTT] topic: ");
  Serial.println(topic);

  Serial.print("[MQTT] payload: ");
  Serial.write(payload, length);
  Serial.println();

  if(length == 0) {
    Serial.println("[MQTT] empty payload ignored");
    return;
  }

  if(strcmp(topic, kTopicPage1) != 0) return;
  static char buf[1024];
  size_t n = (length < sizeof(buf) - 1) ? length : (sizeof(buf) - 1);
  memcpy(buf, payload, n);
  buf[n] = '\0';

  Serial.printf("[MQTT] forwarding to UI, n=%u\n", (unsigned)n);
  lvgl_port_on_ui_json(buf, n);
  Serial.printf("[MQTT] rx topic=%s len=%u\n", topic, (unsigned)length);

}
