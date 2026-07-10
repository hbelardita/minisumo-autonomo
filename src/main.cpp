#include <Arduino.h>

// TB6612FNG Pins
const int STBY = 8;
const int AIN1 = 7;
const int AIN2 = 6;
const int PWMA = 5;
const int BIN1 = 4;
const int BIN2 = 2;
const int PWMB = 9;

// HC-SR04 Pins
const int PIN_TRIG = 11;
const int PIN_ECHO = 12;

// TCRT5000 Pins
const int PIN_TCRT_LEFT = A2;
const int PIN_TCRT_RIGHT = A3;

// UI Pins
const int PIN_BUTTON = 10;
const int PIN_LED = 13;

void setup() {
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    digitalWrite(PIN_LED, LOW);
}

void loop() {
    // Basic sanity blink
    digitalWrite(PIN_LED, HIGH);
    delay(100);
    digitalWrite(PIN_LED, LOW);
    delay(100);
}
