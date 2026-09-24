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
    static unsigned long lastReadTime = 0;
    static unsigned int lastDistance = 0;

    // Respetar un intervalo mínimo (50ms) para evitar solapamiento de ecos
    if (millis() - lastReadTime < 50) {
        return lastDistance;
    }
    lastReadTime = millis();

    // Antes de enviar pulso TRIG, destrabar ECHO si quedó en HIGH
    if (digitalRead(PIN_ECHO) == HIGH) {
        pinMode(PIN_ECHO, OUTPUT);
        digitalWrite(PIN_ECHO, LOW);
        delayMicroseconds(10);
        pinMode(PIN_ECHO, INPUT);
    }

    // Genera pulso de disparo de 10us
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    // Lee la duración del pulso en Echo
    unsigned long duration = pulseIn(PIN_ECHO, HIGH, ULTRASONIC_TIMEOUT);
    if (duration == 0) {
        // Destrabe a PIN_ECHO para evitar colgado de hardware en clones HC-SR04
        pinMode(PIN_ECHO, OUTPUT);
        digitalWrite(PIN_ECHO, LOW);
        delayMicroseconds(10);
        pinMode(PIN_ECHO, INPUT);
        lastDistance = 0;
    } else {
        lastDistance = duration / 58;
    }
    return lastDistance;
}

// Lecturas analógicas directas de los sensores de línea
int lineReadLeftAnalog() {
    return analogRead(PIN_TCRT_LEFT);
}

int lineReadRightAnalog() {
    return analogRead(PIN_TCRT_RIGHT);
}

// Función auxiliar interna para aplicar filtro antirrebote (debounce) a los TCRT5000
static bool readLineSensor(uint8_t pin, int &count) {
    int val = analogRead(pin);
    bool detected = LINE_IS_WHITE_LOW ? ((unsigned int)val < LINE_THRESHOLD_ANALOG) : ((unsigned int)val > LINE_THRESHOLD_ANALOG);
    count = detected ? count + 1 : 0;
    if (count > 2) count = 2;
    return count >= 2;
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

// Lecturas directas sin debounce para chequeos inmediatos
bool lineReadLeftRaw() {
    int val = lineReadLeftAnalog();
    return LINE_IS_WHITE_LOW ? ((unsigned int)val < LINE_THRESHOLD_ANALOG) : ((unsigned int)val > LINE_THRESHOLD_ANALOG);
}

bool lineReadRightRaw() {
    int val = lineReadRightAnalog();
    return LINE_IS_WHITE_LOW ? ((unsigned int)val < LINE_THRESHOLD_ANALOG) : ((unsigned int)val > LINE_THRESHOLD_ANALOG);
}

// Comprueba estado del botón (activo-bajo)
bool buttonPressed() {
    return digitalRead(PIN_BUTTON) == LOW;
}
