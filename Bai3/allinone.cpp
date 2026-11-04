#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* broker = "host.wokwi.internal";

const char* publishTopic = "aet2022/nhom-06/telemetry";
const char* subscribeTopic = "aet2022/nhom-06/cmd/led";

bool ledState = false;

const int LED_PIN = 14;

WiFiClient wifi;
PubSubClient mqtt(wifi);

String clientId;
unsigned long lastSend = 0;

// Test values: replace with actual sensor readings.
float temperature() {
    return 27.0;
}

float humidity() {
    return 55.0;
}

// Receive LED commands.
void callback(char* topic, byte* payload, unsigned int length) {
    if (length != 1) return;

    if (payload[0] == '1') {
        ledState = true;
    } else if (payload[0] == '0') {
        ledState = false;
    } else {
        return;
    }

    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    Serial.println(ledState ? "LED ON" : "LED OFF");
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWi-Fi connected");

    clientId = "nhom06-" + WiFi.macAddress();

    mqtt.setServer(broker, 1883);
    mqtt.setCallback(callback);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) return;

    // Connect and subscribe again after any disconnection.
    if (!mqtt.connected()) {
        if (mqtt.connect(clientId.c_str())) {
            mqtt.subscribe(subscribeTopic);
            Serial.println("MQTT connected");
        } else {
            Serial.print("MQTT failed: ");
            Serial.println(mqtt.state());
            delay(1000);
            return;
        }
    }

    // Process incoming LED commands.
    mqtt.loop();

    // Publish sensor JSON every 2 seconds.
    if (millis() - lastSend >= 2000) {
        lastSend = millis();

        JsonDocument doc;
        doc["temp"] = temperature();
        doc["hum"] = humidity();
        doc["uptime"] = millis() / 1000;
        doc["led"] = ledState;

        String message;
        serializeJson(doc, message);

        if (mqtt.publish(publishTopic, message.c_str())) {
            Serial.println(message);
        } else {
            Serial.println("Publish failed");
        }
    }
}
