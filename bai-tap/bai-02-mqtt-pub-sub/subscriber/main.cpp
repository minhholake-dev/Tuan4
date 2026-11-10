#include <Arduino.h>
#include <ArduinoJson.h>
#include <cmath>
#include <cstring>
#include "mqtt_runtime.h"

MqttRuntime network;

void callback(char* topic, uint8_t* payload, unsigned int length) {
  if (strcmp(topic, local_config::kTelemetryTopic) != 0) return;
  if (length == 0 || length > 512) {
    Serial.println("Telemetry payload length is invalid.");
    return;
  }
  JsonDocument document;
  const DeserializationError error = deserializeJson(
      document, static_cast<const uint8_t*>(payload), length,
      DeserializationOption::NestingLimit(3));
  if (error || !document.is<JsonObject>() ||
      !document["temperature"].is<float>() ||
      !document["humidity"].is<float>() ||
      !document["uptime"].is<uint32_t>() ||
      !document["measurement"].is<const char*>()) {
    Serial.println("Telemetry JSON does not match the expected schema.");
    return;
  }
  const float temperature = document["temperature"].as<float>();
  const float humidity = document["humidity"].as<float>();
  const uint32_t uptime = document["uptime"].as<uint32_t>();
  const char* measurement = document["measurement"].as<const char*>();
  const bool rawAdc = strcmp(measurement, "adc_raw") == 0;
  const bool simulated = strcmp(measurement, "simulated") == 0;
  if (!std::isfinite(temperature) || !std::isfinite(humidity) ||
      (!rawAdc && !simulated) ||
      (rawAdc && (temperature < 0 || temperature > 4095 || humidity < 0 || humidity > 4095)) ||
      (simulated && (temperature < -40 || temperature > 125 || humidity < 0 || humidity > 100))) {
    Serial.println("Telemetry values are out of range or have unknown units.");
    return;
  }
  if (rawAdc) {
    Serial.printf("ADC temperature input: %.0f, ADC humidity input: %.0f\n",
                  temperature, humidity);
  } else {
    Serial.printf("Simulated temperature: %.1f degC, humidity: %.1f %%\n",
                  temperature, humidity);
  }
  Serial.printf("Publisher uptime: %lu s\n", static_cast<unsigned long>(uptime));
}

void setup() {
  Serial.begin(115200);
  network.begin("week4-subscriber", callback, local_config::kTelemetryTopic);
}

void loop() {
  network.poll();
  delay(1);
}
