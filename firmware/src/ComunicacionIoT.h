#ifndef COMUNICACION_IOT_H
#define COMUNICACION_IOT_H

#include <Arduino.h>

// Declaración de la función callback para recibir comandos externos
typedef void (*MqttCallback)(char* topic, byte* payload, unsigned int length);

void inicializarIoT(MqttCallback callback);
void procesarIoT();
void publicarDatos(float humedad, float temperatura, int velocidadMotor, bool tapaAbierta, int modo);

#endif