#ifndef SERVO_TAPA_H
#define SERVO_TAPA_H

#include <Arduino.h>

const int PIN_SERVO = 18;

// ---------------- ÁNGULOS REALES ----------------
// ABIERTA = 0°
// CERRADA = 100°
const int ANGULO_TAPA_ABIERTA = 0;
const int ANGULO_TAPA_CERRADA = 100;

// ---------------- FUNCIONES ----------------
void inicializarServo();

void moverTapa(int grados);

void actualizarServo();

bool controlarTapaAutomatica(
    float humedad,
    bool estadoActual
);

#endif