#ifndef MOTOR_PRINCIPAL_H
#define MOTOR_PRINCIPAL_H

#include <Arduino.h>

const int PIN_MOTOR = 19;
const int TIMER_TOP = 1023; // Resolución de 10 bits (0 - 1023)

void inicializarMotor();
void fijarVelocidadMotor(unsigned int duty);
void controlarMotorAutomatico(float humedad);

#endif