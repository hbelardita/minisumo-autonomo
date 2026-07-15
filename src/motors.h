#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>

// Inicializa los pines del driver de motores
void motorsInit();

// Establece la velocidad de los motores izquierdo y derecho (rango -255 a 255)
void motorsSetSpeed(int leftSpeed, int rightSpeed);

// Aplica freno activo cortocircuitando los motores
void motorsBrake();

// Desactiva el driver (standby) para permitir el movimiento libre (coasting) y ahorrar energía
void motorsStandby();

#endif
