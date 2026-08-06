#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Inicializa los pines de los sensores (Trigger, Echo, TCRT y Botón)
void sensorsInit();

// Lee la distancia con el ultrasónico y devuelve el valor en cm (0 si hay timeout)
unsigned int opponentDistanceRead();

// Verifica el estado del sensor de línea izquierdo con filtro antirrebote
bool borderReadLeft();

// Verifica el estado del sensor de línea derecho con filtro antirrebote
bool borderReadRight();

// Devuelve verdadero si el botón físico está presionado (filtro activo-bajo)
bool buttonPressed();

#endif
