#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pines del Driver de Motores (TB6612FNG)
extern const uint8_t PIN_STBY;
extern const uint8_t PIN_AIN1;
extern const uint8_t PIN_AIN2;
extern const uint8_t PIN_PWMA;
extern const uint8_t PIN_BIN1;
extern const uint8_t PIN_BIN2;
extern const uint8_t PIN_PWMB;

// Pines del Sensor Ultrasónico (HC-SR04)
extern const uint8_t PIN_TRIG;
extern const uint8_t PIN_ECHO;

// Pines de Sensores de Línea (TCRT5000)
extern const uint8_t PIN_TCRT_LEFT;
extern const uint8_t PIN_TCRT_RIGHT;

// Pines de la Interfaz de Usuario (UI)
extern const uint8_t PIN_BUTTON;
extern const uint8_t PIN_LED;

// Umbrales y Tiempos de Calibración
const unsigned int ATTACK_DISTANCE = 50;   // Distancia en cm para atacar
const unsigned long OPENING_MOVE_MS = 400;       // Tiempo de giro inicial de la táctica
const unsigned long RECOVERY_BACKUP_MS = 250;       // Tiempo de retroceso en evasión (con motores TT retrocede ~10cm)
const unsigned long RECOVERY_MS = 450;        // Tiempo total de evasión (retroceso + giro). OJO: Motores TT tienen poco torque, si no llega a girar subir a 550 o 600ms.
const unsigned long PERSIST_MS = 200;      // Tiempo de persistencia de ataque sin ver al rival
const unsigned long COUNTDOWN_MS = 5000;      // Tiempo de espera de seguridad inicial (5 segundos)
const unsigned long BLINK_MS = 100;        // Frecuencia de parpadeo del LED (5Hz)
const unsigned long ULTRASONIC_TIMEOUT = 3500; // Timeout de pulseIn (microsegundos) para ~60cm

// Velocidades de Motores
const int RECOVERY_BACKUP_SPEED = -180;
const int RECOVERY_SPIN_SPEED = 180;
const int OPENING_MOVE_SPEED = 160;
const int SEARCH_SPEED = 120;
const int CHARGE_SPEED = 255;

#endif
