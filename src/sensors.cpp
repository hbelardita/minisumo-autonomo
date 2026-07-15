#include "sensors.h"
#include "config.h"

// Configuración de modos de pin para los sensores
void sensorsInit() {
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    pinMode(PIN_TCRT_LEFT, INPUT);
    pinMode(PIN_TCRT_RIGHT, INPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    
    digitalWrite(PIN_TRIG, LOW);
}

// Envía un pulso ultrasónico y mide el tiempo de respuesta
unsigned int distanceRead() {
    // Genera pulso de disparo de 10us
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    // Lee la duración del pulso en Echo
    unsigned long duration = pulseIn(PIN_ECHO, HIGH, ULTRASONIC_TIMEOUT);
    if (duration == 0) {
        return 0; // Sin obstáculo detectado en el rango configurado
    }
    return duration / 58;
}

// Función auxiliar interna para aplicar filtro antirrebote (debounce) a los TCRT5000
static bool readLineSensor(uint8_t pin, int &count) {
    count = (digitalRead(pin) == LOW) ? count + 1 : 0;
    if (count > 4) count = 4;
    return count >= 4;
}

// Lectura con debounce del sensor izquierdo
bool lineReadLeft() {
    static int count = 0;
    return readLineSensor(PIN_TCRT_LEFT, count);
}

// Lectura con debounce del sensor derecho
bool lineReadRight() {
    static int count = 0;
    return readLineSensor(PIN_TCRT_RIGHT, count);
}

// Comprueba estado del botón (activo-bajo)
bool buttonPressed() {
    return digitalRead(PIN_BUTTON) == LOW;
}
