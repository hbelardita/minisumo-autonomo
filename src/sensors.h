#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

extern const uint8_t PIN_TRIG;
extern const uint8_t PIN_ECHO;
extern const uint8_t PIN_TCRT_LEFT;
extern const uint8_t PIN_TCRT_RIGHT;
extern const uint8_t PIN_BUTTON;

const unsigned long ULTRASONIC_TIMEOUT = 3500;

inline void sensorsInit() {
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    pinMode(PIN_TCRT_LEFT, INPUT);
    pinMode(PIN_TCRT_RIGHT, INPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    
    digitalWrite(PIN_TRIG, LOW);
}

// Returns distance in cm, or 0 if timeout (no object within ~1m)
inline unsigned int distanceRead() {
    // Send 10 microsecond trigger pulse
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    // Read pulse duration
    // Reduce el tiempo bloqueante y es suficiente para un dohyo de 77cm.
    unsigned long duration = pulseIn(PIN_ECHO, HIGH, ULTRASONIC_TIMEOUT);
    if (duration == 0) {
        return 0; // No obstacle in range
    }
    return duration / 58;
}

inline bool readLineSensor(uint8_t pin, int &consecutiveCount) {
    if (digitalRead(pin) == LOW) {
        consecutiveCount++;
        if (consecutiveCount >= 4) {
            consecutiveCount = 4;
            return true;
        }
    } else {
        consecutiveCount = 0;
    }
    return false;
}

inline bool lineReadLeft() {
    static int consecutiveLow = 0;
    return readLineSensor(PIN_TCRT_LEFT, consecutiveLow);
}

inline bool lineReadRight() {
    static int consecutiveLow = 0;
    return readLineSensor(PIN_TCRT_RIGHT, consecutiveLow);
}

inline bool buttonPressed() {
    return digitalRead(PIN_BUTTON) == LOW;
}

#endif
