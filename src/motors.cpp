#include "motors.h"
#include "config.h"

// Inicialización de los pines de motor como salidas
void motorsInit() {
    pinMode(PIN_STBY, OUTPUT);
    pinMode(PIN_AIN1, OUTPUT);
    pinMode(PIN_AIN2, OUTPUT);
    pinMode(PIN_PWMA, OUTPUT);
    pinMode(PIN_BIN1, OUTPUT);
    pinMode(PIN_BIN2, OUTPUT);
    pinMode(PIN_PWMB, OUTPUT);
    
    // Desactiva el driver por defecto
    digitalWrite(PIN_STBY, LOW);
}

// Función auxiliar interna para configurar cada motor
static void setMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int speed) {
    speed = constrain(speed, -255, 255);
    if (speed > 0) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        analogWrite(pwm, speed);
    } else if (speed < 0) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        analogWrite(pwm, -speed);
    } else {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        analogWrite(pwm, 0);
    }
}

// Establece la velocidad de ambos motores
void motorsSetSpeed(int leftSpeed, int rightSpeed) {
    digitalWrite(PIN_STBY, HIGH); // Activa el driver
    setMotor(PIN_AIN1, PIN_AIN2, PIN_PWMA, leftSpeed);
    setMotor(PIN_BIN1, PIN_BIN2, PIN_PWMB, rightSpeed);
}

// Freno activo (corto circuito a VCC/GND para frenar rápido)
void motorsBrake() {
    digitalWrite(PIN_STBY, HIGH);
    digitalWrite(PIN_AIN1, HIGH);
    digitalWrite(PIN_AIN2, HIGH);
    analogWrite(PIN_PWMA, 0);
    digitalWrite(PIN_BIN1, HIGH);
    digitalWrite(PIN_BIN2, HIGH);
    analogWrite(PIN_PWMB, 0);
}

// Desactiva el pin STBY para dejar los motores sueltos
void motorsStandby() {
    digitalWrite(PIN_STBY, LOW);
}
