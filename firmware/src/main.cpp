/* 
 * Control de Convertidor Boost - Adaptado para Arduino Uno
 * Frecuencia: 50kHz | Duty Cycle: 54%
 * Pin de salida: Pin 9 (OC1A)
 */

#include <Arduino.h>

const int mosfetPin = 9; 

void setup() {
  pinMode(mosfetPin, OUTPUT);

  // Configuración del Timer 1 para PWM rápido (Fast PWM)
  // 1. Limpiar registros de control
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // 2. Configurar modo Fast PWM con ICR1 como TOP (Modo 14)
  // WGM13=1, WGM12=1, WGM11=1, WGM10=0
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12);

  // 3. Configurar salida no invertida en el Canal A (Pin 9)
  TCCR1A |= (1 << COM1A1);

  // 4. Establecer la frecuencia a 50kHz
  // Fórmula: F_pwm = F_clk / (Prescaler * (1 + TOP))
  // Con Prescaler = 1: 16,000,000 / (1 * 50,000) = 320. 
  // El valor de TOP es 320 - 1 = 319.
  ICR1 = 319;

  // 5. Establecer el Duty Cycle al 50%
  // 54% de 319 (valor de ICR1) es aproximadamente 172
  OCR1A = 172;

  // 6. Iniciar el Timer con Prescaler = 1 (Sin división)
  TCCR1B |= (1 << CS10);
}

void loop() {
  // El hardware genera la señal de forma autónoma.
}