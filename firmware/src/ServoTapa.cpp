#include "ServoTapa.h"

// ---------------- CALIBRACIÓN ----------------
const int PULSO_MIN_REAL = 620;
const int PULSO_MAX_REAL = 2250;

// ---------------- VARIABLES PWM ----------------
volatile long tiempoPulsoActual = PULSO_MIN_REAL;

long pulsoObjetivo = PULSO_MIN_REAL;

unsigned long servoUltimoCambio = 0;

bool estadoPinServo = LOW;

// ---------------- RAMPA ----------------
unsigned long tiempoUltimoPasoRampa = 0;

const unsigned long velocidadServoMs = 15;

const int pasoMicrosegundos = 5;

// =====================================================
// INICIALIZAR
// =====================================================
void inicializarServo() {

  pinMode(PIN_SERVO, OUTPUT);

  digitalWrite(PIN_SERVO, LOW);

  servoUltimoCambio = micros();
}

// =====================================================
// MOVER TAPA
// =====================================================
void moverTapa(int grados) {

  grados = constrain(grados, 0, 100);

  pulsoObjetivo = map(
      grados,
      100,
      0,
      PULSO_MIN_REAL,
      PULSO_MAX_REAL
  );

  Serial.print("[SERVO] Angulo -> ");
  Serial.println(grados);
}

// =====================================================
// ACTUALIZAR SERVO
// =====================================================
void actualizarServo() {

  unsigned long tiempoActualMs = millis();

  // ---------------- RAMPA SUAVE ----------------
  if (tiempoActualMs - tiempoUltimoPasoRampa >= velocidadServoMs) {

    tiempoUltimoPasoRampa = tiempoActualMs;

    if (tiempoPulsoActual < pulsoObjetivo) {

      tiempoPulsoActual += pasoMicrosegundos;

      if (tiempoPulsoActual > pulsoObjetivo) {
        tiempoPulsoActual = pulsoObjetivo;
      }
    }
    else if (tiempoPulsoActual > pulsoObjetivo) {

      tiempoPulsoActual -= pasoMicrosegundos;

      if (tiempoPulsoActual < pulsoObjetivo) {
        tiempoPulsoActual = pulsoObjetivo;
      }
    }
  }

  // ---------------- PWM CONTINUO ----------------
  unsigned long microsActuales = micros();

  long tiempoEspera = estadoPinServo
                        ? tiempoPulsoActual
                        : (20000 - tiempoPulsoActual);

  if (microsActuales - servoUltimoCambio >= tiempoEspera) {

    estadoPinServo = !estadoPinServo;

    digitalWrite(PIN_SERVO, estadoPinServo);

    servoUltimoCambio = microsActuales;
  }
}

// =====================================================
// CONTROL AUTOMÁTICO
// =====================================================
bool controlarTapaAutomatica(
    float humedad,
    bool estadoActual
) {

  // Mucha humedad -> abrir
  if (humedad >= 75.0 && !estadoActual) {

    moverTapa(ANGULO_TAPA_ABIERTA);

    return true;
  }

  // Humedad normal -> cerrar
  if (humedad < 75.0 && estadoActual) {

    moverTapa(ANGULO_TAPA_CERRADA);

    return false;
  }

  return estadoActual;
}