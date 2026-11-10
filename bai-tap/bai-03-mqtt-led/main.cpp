#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>
#include "board_config.h"
#include "mqtt_runtime.h"

MqttRuntime network;
bool ledState = false;
uint32_t lastSend = 0;

void callback(char* topic, uint8_t* payload, unsigned int length) {
  if (strcmp(topic, local_config::kLedCommandTopic) != 0 || length != 1) return;
  if (payload[0] == '1') {
    ledState = true;
  } else if (payload[0] == '0') {
    ledState = false;
  } else {
    return;
  }
  digitalWrite(board_config::kLedPin, ledState ? HIGH : LOW);
  Serial.println(ledState ? "LED ON" : "LED OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(board_config::kLedPin, OUTPUT);
  digitalWrite(board_config::kLedPin, LOW);
  network.begin("week4-led", callback, local_config::kLedCommandTopic);
}

void loop() {
  const bool connected = network.poll();
  const uint32_t now = millis();
  if (connected && now - lastSend >= board_config::kPublishIntervalMs) {
    lastSend = now;
    JsonDocument document;
    // Demonstration data; replace with calibrated sensor readings.
    document["temperature"] = 27.0;
    document["humidity"] = 55.0;
    document["measurement"] = "simulated";
    document["uptime"] = now / 1000U;
    document["led"] = ledState;
    Serial.println(network.publish(document)
                       ? "Simulated telemetry published."
                       : "Telemetry publish failed.");
  }
  delay(1);
}
