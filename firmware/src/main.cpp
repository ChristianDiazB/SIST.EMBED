#include <Arduino.h>

// userofagromist 12345678aA


#include <WiFi.h>

#include <PubSubClient.h>

#include <WiFiClientSecure.h>

const char* ssid = "FAMILIA_DIAZ";
const char* password = "tata2026";

const char* mqtt_server = "0f28df32393840c09e3e44067bb69677.s1.eu.hivemq.cloud";

const int mqtt_port = 8883;

const char* mqtt_user = "userofagromist";
const char* mqtt_pass = "12345678aA";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void reconnectMQTT() {

    while (!client.connected()) {

        Serial.println("Conectando MQTT...");

        if (client.connect(
            "ESP32Client",
            mqtt_user,
            mqtt_pass
        )) {

            Serial.println("MQTT conectado");

        } else {

            Serial.print("Error MQTT: ");
            Serial.println(client.state());

            delay(3000);
        }
    }
}

void setup() {

    Serial.begin(115200);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {

        delay(500);
        Serial.println("Conectando WiFi...");
    }

    Serial.println("WiFi conectado");

    espClient.setInsecure();

    client.setServer(mqtt_server, mqtt_port);
}

void loop() {

    if (!client.connected()) {
        reconnectMQTT();
    }

    client.loop();

    client.publish(
        "agromist/test",
        "hola desde esp32"
    );

    Serial.println("Mensaje enviado");

    delay(5000);
}