#include <Arduino.h>
#include "motors.h"

// TB6612FNG Pins
extern const uint8_t STBY = 8;
extern const uint8_t AIN1 = 7;
extern const uint8_t AIN2 = 6;
extern const uint8_t PWMA = 5;
extern const uint8_t BIN1 = 4;
extern const uint8_t BIN2 = 2;
extern const uint8_t PWMB = 9;

// HC-SR04 Pins
constexpr uint8_t PIN_TRIG = 11;
constexpr uint8_t PIN_ECHO = 12;

// TCRT5000 Pins
constexpr uint8_t PIN_TCRT_LEFT = A2;
constexpr uint8_t PIN_TCRT_RIGHT = A3;

// UI Pins
constexpr uint8_t PIN_BUTTON = 10;
constexpr uint8_t PIN_LED = 13;

void setup() {
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    digitalWrite(PIN_LED, LOW);
    motorsInit();
}

void loop() {
    // Test motor spin forward briefly
    motorsSetSpeed(100, 100);
    delay(500);
    motorsBrake();
    delay(1000);
}
