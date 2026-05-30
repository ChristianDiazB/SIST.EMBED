#include "MotorPrincipal.h"

const int FREQ_MOTOR = 10000;
const int RES_MOTOR = 10;

void inicializarMotor() {

  ledcAttach(PIN_MOTOR, FREQ_MOTOR, RES_MOTOR);

  fijarVelocidadMotor(0);
}

void fijarVelocidadMotor(unsigned int duty) {

  duty = constrain(duty, 0, TIMER_TOP);

  ledcWrite(PIN_MOTOR, duty);
}