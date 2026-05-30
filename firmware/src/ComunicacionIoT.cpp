#include "ComunicacionIoT.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// --- CREDENCIALES DE RED Y BROKER (Completa con tus datos de HiveMQ) ---
const char* ssid        = "USTA_Estudiantes";
const char* password    = "#soytomasino";
const char* mqtt_server = "0f28df32393840c09e3e44067bb69677.s1.eu.hivemq.cloud"; 
const int mqtt_port     = 8883;
const char* mqtt_user   = "userofagromist";
const char* mqtt_pass   = "12345678aA";

WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long ultimoIntentoReconexion = 0;
const unsigned long intervaloReconexion = 5000; // Intentar conectar cada 5s sin bloquear

void verificarConexionMQTT() {
  if (client.connected()) return;

  unsigned long tiempoActual = millis();
  if (tiempoActual - ultimoIntentoReconexion >= intervaloReconexion) {
    ultimoIntentoReconexion = tiempoActual;
    Serial.print("Intentando conexión MQTT asíncrona...");
    
    // Generamos un ID de cliente único basado en la MAC del ESP32
    String clientID = "ESP32_AgroMist_" + String((uint32_t)ESP.getEfuseMac(), HEX);
    
    if (client.connect(clientID.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("¡Conectado a HiveMQ Cloud!");
      // Nos suscribimos a los tópicos de control remoto de Node-RED
      client.subscribe("agromist/cmd/modo");
      client.subscribe("agromist/cmd/bomba");
      client.subscribe("agromist/cmd/tapa");
    } else {
      Serial.print("Falló, código de error: ");
      Serial.println(client.state());
    }
  }
}

void inicializarIoT(MqttCallback callback) {
  Serial.println("\nConectando Wi-Fi...");
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  
  // Un pequeño bucle inicial de espera para asegurar el arranque
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Conectado con éxito.");
    Serial.print("Dirección IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi en segundo plano (reconectando de forma asíncrona)...");
  }

  // Configuración de Seguridad TLS Obligatoria para HiveMQ Cloud
  espClient.setInsecure(); 
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void procesarIoT() {
  // Si el Wi-Fi se cae, el core del ESP32 intenta reconectar solo gracias a WiFi.begin()
  if (WiFi.status() == WL_CONNECTED) {
    verificarConexionMQTT();
    client.loop();
  }
}

void publicarDatos(float humedad, float temperatura, int velocidadMotor, bool tapaAbierta, int modo) {
  if (!client.connected()) return;

  // Agregamos la propiedad "temp" al JSON
  String jsonTelemetria = "{\"humedad\":" + String(humedad, 1) + 
                          ",\"temp\":" + String(temperatura, 1) + 
                          ",\"bomba\":" + String(velocidadMotor) + 
                          ",\"tapa\":" + String(tapaAbierta ? 1 : 0) + 
                          ",\"modo\":" + String(modo) + "}";

  client.publish("agromist/telemetria", jsonTelemetria.c_str());
}