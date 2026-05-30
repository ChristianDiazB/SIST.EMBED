#include "Sensores.h"
#include <DHT.h>

#define DHTTYPE DHT11

DHT dht(PIN_DHT, DHTTYPE);

float humedadCache = 0;
float temperaturaCache = 0;

void inicializarSensores() {

  dht.begin();

  pinMode(PIN_HIH, INPUT);
}

void actualizarSensores() {

  float humedadDHT = dht.readHumidity();

  float temperatura = dht.readTemperature();

  int lecturaADC = analogRead(PIN_HIH);

  float voltaje = lecturaADC * (3.3 / 4095.0);

  float humedadHIH =
      (voltaje / 3.3 - 0.1515) / 0.00636;

  humedadHIH = constrain(humedadHIH, 0, 100);

  if (isnan(humedadDHT)) {

    humedadDHT = humedadHIH;
  }

  humedadCache =
      (humedadDHT + humedadHIH) / 2.0;

  if (!isnan(temperatura)) {

    temperaturaCache = temperatura;
  }
}

float obtenerHumedad() {

  return humedadCache;
}

float obtenerTemperatura() {

  return temperaturaCache;
}