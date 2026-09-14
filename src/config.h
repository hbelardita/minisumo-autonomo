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
const unsigned int ATTACK_DISTANCE = 15;           // Distancia en cm para atacar (cuerpo a cuerpo)
const unsigned int ATTACK_RELEASE_DISTANCE = 20;   // Distancia en cm para salir de ataque hacia aproximación (histéresis)
const unsigned int APPROACH_DISTANCE = 70;         // Distancia en cm para aproximación (cubre oponentes en dohyo de 77 cm)
const unsigned long TACTIC_MS = 400;       // Tiempo de giro inicial de la táctica
const unsigned long BACKUP_MS = 800;       // Tiempo de retroceso en evasión
const unsigned long PERSIST_MS = 1500;      // Tiempo de persistencia de ataque sin ver al rival
const unsigned long SAFETY_MS = 5000;      // Tiempo de espera de seguridad inicial (5 segundos)
const unsigned long BLINK_MS = 100;        // Frecuencia de parpadeo del LED (5Hz)
const unsigned long ULTRASONIC_TIMEOUT = 5000; // Timeout de pulseIn en us (~70cm util + overhead ~450us del HC-SR04)

// Velocidades de Motores
const int EVADE_BACKUP_SPEED_L = -180; // Ajustar si curva a la derecha
const int EVADE_BACKUP_SPEED_R = -180; // Ajustar si curva a la izquierda
const int EVADE_SPIN_SPEED = 200;
const int INITIAL_TACTIC_SPEED = 160;
const int SEARCH_SPEED = 120;
const int APPROACH_SPEED = 160;
const int ATTACK_SPEED = 255;

#endif
