#include <Arduino.h>
#include <esp_task_wdt.h>

#include "Sensores.h"
#include "MotorPrincipal.h"
#include "ServoTapa.h"
#include "ComunicacionIoT.h"

// =====================================================
// VARIABLES MQTT
// =====================================================
String ultimoComandoTapa = "";
String ultimoComandoBomba = "";
String ultimoComandoModo = "";

// =====================================================
// BOTONES
// =====================================================
const int btnServoPin  = 14;
const int btnRiegoPin  = 27;
const int btnAutoPin   = 26;
const int btnManualPin = 25;

// =====================================================
// TIEMPOS
// =====================================================
unsigned long tiempoAnterior = 0;

const unsigned long intervaloLectura = 2000;

unsigned long tiempoAnteriorMQTT = 0;

const unsigned long intervaloMQTT = 3000;

const unsigned long retrasoDebounce = 50;

// =====================================================
// ESTADOS
// =====================================================
bool estadoTapaAbierta = false;

bool estadoBombaActiva = false;

bool ultimoEstadoBtnServo = HIGH;

bool ultimoEstadoBtnRiego = HIGH;

bool ultimoEstadoBtnAuto = HIGH;

bool ultimoEstadoBtnManual = HIGH;

unsigned long tiempoDebounceServo = 0;

unsigned long tiempoDebounceRiego = 0;

unsigned long tiempoDebounceModos = 0;

int velocidadActualMotor = 0;

// =====================================================
// MODOS
// =====================================================
enum Modos {
  MODO_AUTOMATICO,
  MODO_MANUAL
};

Modos modoSistema = MODO_AUTOMATICO;

// =====================================================
// CALLBACK MQTT
// =====================================================
void callbackMQTT(
    char* topic,
    byte* payload,
    unsigned int length
) {

  String mensaje = "";

  for (unsigned int i = 0; i < length; i++) {

    mensaje += (char)payload[i];
  }

  Serial.print("\n[MQTT] ");
  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(mensaje);

  // =================================================
  // CAMBIO DE MODO
  // =================================================
  if (String(topic) == "agromist/cmd/modo") {

    if (mensaje == ultimoComandoModo)
      return;

    ultimoComandoModo = mensaje;

    if (mensaje == "AUTO") {

      modoSistema = MODO_AUTOMATICO;

      Serial.println("Modo AUTOMÁTICO");
    }
    else if (mensaje == "MANUAL") {

      modoSistema = MODO_MANUAL;

      ultimoEstadoBtnRiego = HIGH;

      Serial.println("Modo MANUAL");
    }
  }

  // SOLO MANUAL
  if (modoSistema != MODO_MANUAL)
    return;

  // =================================================
  // BOMBA
  // =================================================
  if (String(topic) == "agromist/cmd/bomba") {

    if (mensaje == ultimoComandoBomba)
      return;

    ultimoComandoBomba = mensaje;

    if (mensaje == "ON") {

      estadoBombaActiva = true;

      velocidadActualMotor = TIMER_TOP / 2;

      fijarVelocidadMotor(velocidadActualMotor);

      Serial.println("Bomba ON");
    }
    else if (mensaje == "OFF") {

      estadoBombaActiva = false;

      velocidadActualMotor = 0;

      fijarVelocidadMotor(0);

      Serial.println("Bomba OFF");
    }
  }

  // =================================================
  // TAPA
  // =================================================
  if (String(topic) == "agromist/cmd/tapa") {

    if (mensaje == ultimoComandoTapa)
      return;

    ultimoComandoTapa = mensaje;

    if (mensaje == "OPEN") {

      estadoTapaAbierta = true;

      moverTapa(ANGULO_TAPA_ABIERTA);
    }
    else if (mensaje == "CLOSE") {

      estadoTapaAbierta = false;

      moverTapa(ANGULO_TAPA_CERRADA);
    }
  }
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  // WATCHDOG NUEVO ESP32 CORE 3.x
  esp_task_wdt_config_t wdt_config = {
      .timeout_ms = 10000,
      .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
      .trigger_panic = true
  };

  esp_task_wdt_init(&wdt_config);

  esp_task_wdt_add(NULL);

  inicializarSensores();

  inicializarMotor();

  inicializarServo();

  inicializarIoT(callbackMQTT);

  pinMode(btnServoPin, INPUT_PULLUP);

  pinMode(btnRiegoPin, INPUT_PULLUP);

  pinMode(btnAutoPin, INPUT_PULLUP);

  pinMode(btnManualPin, INPUT_PULLUP);

  moverTapa(ANGULO_TAPA_CERRADA);

  Serial.println("Sistema iniciado");
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  actualizarServo();

  procesarIoT();

  unsigned long tiempoActual = millis();

  // =================================================
  // BOTONES DE MODO
  // =================================================
  int lecturaBtnAuto = digitalRead(btnAutoPin);

  int lecturaBtnManual = digitalRead(btnManualPin);

  if (tiempoActual - tiempoDebounceModos > retrasoDebounce) {

    if (lecturaBtnAuto == LOW &&
        ultimoEstadoBtnAuto == HIGH) {

      modoSistema = MODO_AUTOMATICO;

      Serial.println("Modo AUTOMÁTICO");

      tiempoDebounceModos = tiempoActual;
    }

    if (lecturaBtnManual == LOW &&
        ultimoEstadoBtnManual == HIGH) {

      modoSistema = MODO_MANUAL;

      ultimoEstadoBtnRiego = HIGH;

      Serial.println("Modo MANUAL");

      tiempoDebounceModos = tiempoActual;
    }

    ultimoEstadoBtnAuto = lecturaBtnAuto;

    ultimoEstadoBtnManual = lecturaBtnManual;
  }

  // =================================================
  // ACTUALIZAR SENSORES
  // =================================================
  if (tiempoActual - tiempoAnterior >= intervaloLectura) {

    tiempoAnterior = tiempoActual;

    actualizarSensores();
  }

  float humedad = obtenerHumedad();

  float temperatura = obtenerTemperatura();

  // =================================================
  // MODO AUTOMÁTICO
  // =================================================
  if (modoSistema == MODO_AUTOMATICO) {

    // ---------------- BOMBA ----------------
    if (humedad < 40 && !estadoBombaActiva) {

      estadoBombaActiva = true;
    }
    else if (humedad > 75 && estadoBombaActiva) {

      estadoBombaActiva = false;
    }

    if (estadoBombaActiva) {

      velocidadActualMotor =
          (int)((80.0 - humedad) *
          TIMER_TOP / 40.0);

      velocidadActualMotor =
          constrain(
              velocidadActualMotor,
              0,
              TIMER_TOP
          );
    }
    else {

      velocidadActualMotor = 0;
    }

    fijarVelocidadMotor(velocidadActualMotor);

    // ---------------- TAPA ----------------
    if (temperatura >= 35.0) {

      estadoTapaAbierta = true;

      moverTapa(ANGULO_TAPA_ABIERTA);
    }
    else if (temperatura <= 15.0) {

      estadoTapaAbierta = false;

      moverTapa(ANGULO_TAPA_CERRADA);
    }
    else {

      estadoTapaAbierta =
          controlarTapaAutomatica(
              humedad,
              estadoTapaAbierta
          );
    }
  }

  // =================================================
  // MODO MANUAL
  // =================================================
  else {

    // ---------------- TAPA ----------------
    int lecturaBtnServo =
        digitalRead(btnServoPin);

    if (tiempoActual - tiempoDebounceServo >
        retrasoDebounce) {

      if (lecturaBtnServo == LOW &&
          ultimoEstadoBtnServo == HIGH) {

        estadoTapaAbierta =
            !estadoTapaAbierta;

        moverTapa(
            estadoTapaAbierta
              ? ANGULO_TAPA_ABIERTA
              : ANGULO_TAPA_CERRADA
        );

        tiempoDebounceServo =
            tiempoActual;
      }

      ultimoEstadoBtnServo =
          lecturaBtnServo;
    }

    // ---------------- BOMBA ----------------
    int lecturaBtnRiego =
        digitalRead(btnRiegoPin);

    if (tiempoActual - tiempoDebounceRiego >
        retrasoDebounce) {

      if (lecturaBtnRiego == LOW &&
          ultimoEstadoBtnRiego == HIGH) {

        estadoBombaActiva =
            !estadoBombaActiva;

        if (estadoBombaActiva) {

          velocidadActualMotor =
              TIMER_TOP / 2;
        }
        else {

          velocidadActualMotor = 0;
        }

        fijarVelocidadMotor(
            velocidadActualMotor
        );

        tiempoDebounceRiego =
            tiempoActual;
      }

      ultimoEstadoBtnRiego =
          lecturaBtnRiego;
    }
  }

  // =================================================
  // TELEMETRÍA MQTT
  // =================================================
  if (tiempoActual - tiempoAnteriorMQTT >=
      intervaloMQTT) {

    tiempoAnteriorMQTT = tiempoActual;

    publicarDatos(
        humedad,
        temperatura,
        velocidadActualMotor,
        estadoTapaAbierta,
        (int)modoSistema
    );
  }

  esp_task_wdt_reset();
}