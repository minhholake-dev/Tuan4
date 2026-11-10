#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <cstring>
#include "secrets.h"

class MqttRuntime {
 public:
  using Callback = void (*)(char*, uint8_t*, unsigned int);

  MqttRuntime() : mqtt_(wifi_) {}

  void begin(const char* role, Callback callback = nullptr,
             const char* subscription = nullptr) {
    subscription_ = subscription;
    configured_ = local_config::kWifiSsid[0] != '\0' &&
                  strcmp(local_config::kWifiSsid, "YOUR_WIFI") != 0 &&
                  local_config::kMqttHost[0] != '\0' &&
                  local_config::kMqttPort != 0 &&
                  validTopic(local_config::kTelemetryTopic) &&
                  (subscription_ == nullptr || validTopic(subscription_));
    if (!configured_) {
      Serial.println("Configure include/secrets.h before connecting.");
      return;
    }
    const uint64_t id = ESP.getEfuseMac();
    snprintf(clientId_, sizeof(clientId_), "%s-%04x%08lx", role,
             static_cast<unsigned int>((id >> 32) & 0xffff),
             static_cast<unsigned long>(id & 0xffffffff));
    wifi_.setConnectionTimeout(1500);
    mqtt_.setServer(local_config::kMqttHost, local_config::kMqttPort);
    mqtt_.setSocketTimeout(2);
    mqtt_.setKeepAlive(30);
    if (!mqtt_.setBufferSize(768)) {
      configured_ = false;
      Serial.println("Unable to allocate MQTT packet buffer.");
      return;
    }
    if (callback != nullptr) mqtt_.setCallback(callback);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(local_config::kWifiSsid, local_config::kWifiPassword);
    lastWifiAttempt_ = millis();
    lastMqttAttempt_ = millis() - kMqttRetryMs;
  }

  bool poll() {
    if (!configured_) return false;
    const uint32_t now = millis();
    if (WiFi.status() != WL_CONNECTED) {
      if (mqtt_.connected()) mqtt_.disconnect();
      if (now - lastWifiAttempt_ >= kWifiRetryMs) {
        lastWifiAttempt_ = now;
        WiFi.reconnect();
        Serial.println("Retrying Wi-Fi connection.");
      }
      return false;
    }
    if (!mqtt_.connected()) {
      if (now - lastMqttAttempt_ < kMqttRetryMs) return false;
      lastMqttAttempt_ = now;
      const bool connected = local_config::kMqttUsername[0] == '\0'
                                 ? mqtt_.connect(clientId_)
                                 : mqtt_.connect(clientId_, local_config::kMqttUsername,
                                                 local_config::kMqttPassword);
      if (!connected) {
        Serial.printf("MQTT connection failed (state %d).\n", mqtt_.state());
        return false;
      }
      if (subscription_ != nullptr && !mqtt_.subscribe(subscription_)) {
        Serial.println("MQTT subscription request failed; retrying later.");
        mqtt_.disconnect();
        return false;
      }
      Serial.println("MQTT connected.");
    }
    return mqtt_.loop();
  }

  bool publish(const JsonDocument& document) {
    if (!mqtt_.connected()) return false;
    char payload[512];
    const size_t required = measureJson(document);
    if (document.overflowed() || required >= sizeof(payload)) {
      Serial.println("Telemetry exceeds the JSON payload limit.");
      return false;
    }
    const size_t length = serializeJson(document, payload, sizeof(payload));
    if (length != required) return false;
    return mqtt_.publish(local_config::kTelemetryTopic,
                         reinterpret_cast<const uint8_t*>(payload), length, false);
  }

 private:
  static bool validTopic(const char* topic) {
    return topic != nullptr && topic[0] != '\0' && strlen(topic) <= 160 &&
           strchr(topic, '#') == nullptr && strchr(topic, '+') == nullptr;
  }
  static constexpr uint32_t kWifiRetryMs = 15000;
  static constexpr uint32_t kMqttRetryMs = 5000;
  WiFiClient wifi_;
  PubSubClient mqtt_;
  char clientId_[64] = {};
  const char* subscription_ = nullptr;
  uint32_t lastWifiAttempt_ = 0;
  uint32_t lastMqttAttempt_ = 0;
  bool configured_ = false;
};
