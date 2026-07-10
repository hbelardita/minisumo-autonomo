#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>

extern const uint8_t STBY;
extern const uint8_t AIN1;
extern const uint8_t AIN2;
extern const uint8_t PWMA;
extern const uint8_t BIN1;
extern const uint8_t BIN2;
extern const uint8_t PWMB;

inline void motorsInit() {
    pinMode(STBY, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(PWMA, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(PWMB, OUTPUT);
    
    digitalWrite(STBY, LOW); // Disable driver by default
}

inline void motorsSetSpeed(int leftSpeed, int rightSpeed) {
    leftSpeed = constrain(leftSpeed, -255, 255);
    rightSpeed = constrain(rightSpeed, -255, 255);

    digitalWrite(STBY, HIGH); // Enable driver

    // Left Motor (Motor A)
    if (leftSpeed > 0) {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        analogWrite(PWMA, leftSpeed);
    } else if (leftSpeed < 0) {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        analogWrite(PWMA, -leftSpeed);
    } else {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, LOW);
        analogWrite(PWMA, 0);
    }

    // Right Motor (Motor B)
    if (rightSpeed > 0) {
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
        analogWrite(PWMB, rightSpeed);
    } else if (rightSpeed < 0) {
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
        analogWrite(PWMB, -rightSpeed);
    } else {
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, LOW);
        analogWrite(PWMB, 0);
    }
}

inline void motorsBrake() {
    digitalWrite(STBY, HIGH);
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, 0);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, 0);
}

#endif
