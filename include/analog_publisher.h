#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "board_config.h"
#include "mqtt_runtime.h"

#ifndef WEEK4_PUBLISHER_ROLE
#define WEEK4_PUBLISHER_ROLE "week4-publisher"
#endif

namespace analog_publisher {
MqttRuntime network;
uint32_t lastSend = 0;
}

void setup() {
  Serial.begin(115200);
  pinMode(board_config::kTemperatureAdcPin, INPUT);
  pinMode(board_config::kHumidityAdcPin, INPUT);
  analogReadResolution(12);
  analog_publisher::network.begin(WEEK4_PUBLISHER_ROLE);
}

void loop() {
  const bool connected = analog_publisher::network.poll();
  const uint32_t now = millis();
  if (connected && now - analog_publisher::lastSend >= board_config::kPublishIntervalMs) {
    analog_publisher::lastSend = now;
    JsonDocument document;
    // Raw 12-bit ADC readings, not calibrated degC or humidity %.
    document["temperature"] = analogRead(board_config::kTemperatureAdcPin);
    document["humidity"] = analogRead(board_config::kHumidityAdcPin);
    document["measurement"] = "adc_raw";
    document["uptime"] = now / 1000U;
    if (local_config::kStudentName[0] != '\0') {
      document["student_name"] = local_config::kStudentName;
    }
    if (local_config::kStudentId[0] != '\0') {
      document["student_id"] = local_config::kStudentId;
    }
    Serial.println(analog_publisher::network.publish(document)
                       ? "ADC telemetry published."
                       : "ADC telemetry publish failed.");
  }
  delay(1);
}
