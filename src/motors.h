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

inline void setMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int speed) {
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

inline void motorsSetSpeed(int leftSpeed, int rightSpeed) {
    digitalWrite(STBY, HIGH); // Enable driver
    setMotor(AIN1, AIN2, PWMA, leftSpeed);
    setMotor(BIN1, BIN2, PWMB, rightSpeed);
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
