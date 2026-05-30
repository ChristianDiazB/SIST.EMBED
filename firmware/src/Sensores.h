#ifndef SENSORES_H
#define SENSORES_H

#include <Arduino.h>

const int PIN_DHT = 4;
const int PIN_HIH = 34;

void inicializarSensores();

void actualizarSensores();

float obtenerHumedad();

float obtenerTemperatura();

#endif