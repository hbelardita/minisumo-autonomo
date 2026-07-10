#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

extern const uint8_t PIN_TRIG;
extern const uint8_t PIN_ECHO;
extern const uint8_t PIN_TCRT_LEFT;
extern const uint8_t PIN_TCRT_RIGHT;
extern const uint8_t PIN_BUTTON;

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

    // Read pulse duration with a 6000 microsecond limit (~100cm max range)
    unsigned long duration = pulseIn(PIN_ECHO, HIGH, 6000);
    if (duration == 0) {
        return 0; // No obstacle in range
    }
    return duration / 58;
}

inline int lineReadLeft() {
    return analogRead(PIN_TCRT_LEFT);
}

inline int lineReadRight() {
    return analogRead(PIN_TCRT_RIGHT);
}

inline bool buttonPressed() {
    return digitalRead(PIN_BUTTON) == LOW;
}

#endif
