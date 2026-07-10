#include <Arduino.h>
#include "motors.h"
#include "sensors.h"

// TB6612FNG Pins
extern const uint8_t STBY = 8;
extern const uint8_t AIN1 = 7;
extern const uint8_t AIN2 = 6;
extern const uint8_t PWMA = 5;
extern const uint8_t BIN1 = 4;
extern const uint8_t BIN2 = 2;
extern const uint8_t PWMB = 9;

// HC-SR04 Pins
extern const uint8_t PIN_TRIG = 11;
extern const uint8_t PIN_ECHO = 12;

// TCRT5000 Pins
extern const uint8_t PIN_TCRT_LEFT = A2;
extern const uint8_t PIN_TCRT_RIGHT = A3;

// UI Pins
extern const uint8_t PIN_BUTTON = 10;
extern const uint8_t PIN_LED = 13;

void setup() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    
    motorsInit();
    sensorsInit();
}

void loop() {
    // Test read
    unsigned int dist = distanceRead();
    if (dist > 0 && dist < 50) {
        digitalWrite(PIN_LED, HIGH);
    } else {
        digitalWrite(PIN_LED, LOW);
    }
    delay(50);
}
