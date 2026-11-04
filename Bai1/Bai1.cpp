#include   <Arduino.h>
#include   <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
const int LED_PIN = 22;
const int Humidity_PIN = 34;
const int Temperature_PIN = 35;

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

const char* broker = "test.mosquitto.org";
const char* topic = "bica/tuan4/nhom-06/telemetry";
WiFiClient wifi;

unsigned long lastSend = 0;
const sv = "Tran Minh Ho";
const char* msv = "25119152";

PubSubClient mqtt(wifi);
float temperature() {
  return analogRead(Temperature_PIN);
}
float humidity() {
  return analogRead(Humidity_PIN);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    clientID = "nhom06-tuan4"+WiFi.macAddress();
    mqtt.setServer(broker, 1883);
}
void loop() {
      if (!mqtt.connected(clientID.c_str())) {
        // Use a different client ID for each ESP32.
        if (!mqtt.connect(clientID.c_str())) {
            delay(1000);
            return;
        }
    }

    mqtt.loop();

    if (millis() - lastSend >= 2000) {
        lastSend = millis();

        // 1. Read the sensor through your functions
    float temp = temperature();
    float hum = humidity();

    // 2. Pack the readings into JSON
    JsonDocument doc;
    doc["temperature"] = temp;
    doc["humidity"] = hum;
    doc["uptime"] = millis()/1000;
    doc["sv"] = sv;
    doc["msv"] = msv;

    // 3. Convert JSON to text
    String message;
    serializeJson(doc, message);

    // 4. Publish the text
    if (mqtt.publish(topic, message.c_str())) {
        Serial.println(message);
    } else {
        Serial.println("Publish failed");
    }
    }
}