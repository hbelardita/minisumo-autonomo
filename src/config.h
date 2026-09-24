#ifndef CONFIG_H
#define CONFIG_H

// #define DEBUG_MOTORS // Se habilita mediante build_flags en platformio.ini
// #define DEBUG_SENSORS // Se habilita mediante build_flags en platformio.ini
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

// Constantes de Calibración de Evasión
const unsigned long EVADE_BACKUP_MS = 300; // Tiempo de retroceso al pisar linea (ajustable facilmente)
const unsigned long EVADE_TURN_MS = 150;   // Tiempo de giro al pisar linea (ajustable facilmente)
const int EVADE_BACKUP_SPEED_L = -150;     // Velocidad de retroceso rueda izquierda
const int EVADE_BACKUP_SPEED_R = -150;     // Velocidad de retroceso rueda derecha
const int EVADE_SPIN_SPEED = 200;          // Velocidad de giro durante evasion

// Parámetros de Sensores de Línea (Analógico)
const unsigned int LINE_THRESHOLD_ANALOG = 400; // Umbral analogico para linea blanca (0-1023)
const bool LINE_IS_WHITE_LOW = true; // Si el sensor devuelve LOW / voltaje bajo sobre blanco

// Umbrales y Tiempos de Calibración
const unsigned int ATTACK_DISTANCE = 12;           // Distancia en cm para atacar (cuerpo a cuerpo)
const unsigned int ATTACK_RELEASE_DISTANCE = 25;   // Distancia en cm para salir de ataque hacia aproximación (histéresis)
const unsigned int APPROACH_DISTANCE = 60;         // Distancia en cm para aproximación (cubre oponentes en dohyo de 77 cm)
const unsigned long PERSIST_MS = 1500;      // Tiempo de persistencia de ataque sin ver al rival
const unsigned long SAFETY_MS = 5000;      // Tiempo de espera de seguridad inicial (5 segundos)
const unsigned long BLINK_MS = 100;        // Frecuencia de parpadeo del LED (5Hz)
const unsigned long ULTRASONIC_TIMEOUT = 18000; // Timeout de pulseIn en us (~77cm util + latencia de inicio HC-SR04)

// Velocidades de Motores
const int SEARCH_SPEED = 100;
const int APPROACH_SPEED = 160;
const int RAMP_SPEED_30 = 200;
const int RAMP_SPEED_25 = 220;
const int RAMP_SPEED_18 = 235;
const int ATTACK_SPEED = 255;

#endif
