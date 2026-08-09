#ifndef CONFIG_H
#define CONFIG_H

// #define DEBUG_MOTORS // Se habilita mediante build_flags en platformio.ini
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
const unsigned int ATTACK_DISTANCE = 15;   // Distancia en cm para atacar (cuerpo a cuerpo)
const unsigned int APPROACH_DISTANCE = 35; // Distancia en cm para aproximación (media)
const unsigned long TACTIC_MS = 400;       // Tiempo de giro inicial de la táctica
const unsigned long BACKUP_MS = 450;       // Tiempo de retroceso en evasión
const unsigned long EVADE_MS = 700;        // Tiempo total de evasión (retroceso + giro)
const unsigned long PERSIST_MS = 200;      // Tiempo de persistencia de ataque sin ver al rival
const unsigned long SAFETY_MS = 5000;      // Tiempo de espera de seguridad inicial (5 segundos)
const unsigned long BLINK_MS = 100;        // Frecuencia de parpadeo del LED (5Hz)
const unsigned long ULTRASONIC_TIMEOUT = 3500; // Timeout de pulseIn (microsegundos) para ~60cm

// Velocidades de Motores
const int EVADE_BACKUP_SPEED = -180;
const int EVADE_SPIN_SPEED = 180;
const int INITIAL_TACTIC_SPEED = 160;
const int SEARCH_SPEED = 120;
const int APPROACH_SPEED = 160;
const int ATTACK_SPEED = 255;

#endif
