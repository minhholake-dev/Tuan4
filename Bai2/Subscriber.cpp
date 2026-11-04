#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* broker = "host.wokwi.internal";
const char* topic = "aet2022/nhom-06/telemetry";

WiFiClient wifi;
PubSubClient mqtt(wifi);

String clientId;

void callback(char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;

    if (deserializeJson(doc, payload, length)) {
        Serial.println("Invalid JSON");
        return;
    }

    float temp = doc["temp"];
    float hum = doc["hum"];
    unsigned long uptime = doc["uptime"];

    Serial.print("Temperature: ");
    Serial.println(temp);

    Serial.print("Humidity: ");
    Serial.println(hum);

    Serial.print("Publisher uptime: ");
    Serial.println(uptime);
}

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin("Wokwi-GUEST", "");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWi-Fi connected");

    clientId = "nhom06-sub-" + WiFi.macAddress();

    mqtt.setServer(broker, 1883);
    mqtt.setCallback(callback);
}

void loop() {
    if (!mqtt.connected()) {
        if (mqtt.connect(clientId.c_str())) {
            mqtt.subscribe(topic);
            Serial.println("Subscribed to laptop broker");
        } else {
            Serial.print("MQTT connection failed: ");
            Serial.println(mqtt.state());
            delay(1000);
            return;
        }
    }

    mqtt.loop();
}