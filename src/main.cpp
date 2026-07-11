#include <Arduino.h>
#include "motors.h"
#include "sensors.h"


// #define DEBUG_MOTORS

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

// Calibration Thresholds
const int LINE_THRESHOLD = 400; // Low reading represents white line
const unsigned int ATTACK_DISTANCE = 50; // Threshold in cm to trigger attack
const unsigned long INITIAL_TACTIC_DURATION = 400; // Spin duration in ms

// FSM Configuration Constants
const int EVADE_BACKUP_SPEED = -180;
const int EVADE_SPIN_SPEED = 180;
const unsigned long EVADE_BACKUP_DURATION = 250;
const unsigned long EVADE_TOTAL_DURATION = 450;
const int INITIAL_TACTIC_SPEED = 160;
const int SEARCH_SPEED = 120;
const int ATTACK_SPEED = 255;
const unsigned long SAFETY_DELAY_DURATION = 5000;
const unsigned long LED_BLINK_INTERVAL = 100;

// FSM States
enum State {
    STATE_STANDBY,
    STATE_SAFETY_DELAY,
    STATE_INITIAL_TACTIC,
    STATE_SEARCH,
    STATE_ATTACK,
    STATE_EVADE_LEFT,
    STATE_EVADE_RIGHT
};

State currentState = STATE_STANDBY;
unsigned long stateStartTime = 0;
unsigned long lastLedBlinkTime = 0;
bool ledState = false;

void transitionTo(State newState) {
    currentState = newState;
    stateStartTime = millis();
    if (newState == STATE_SAFETY_DELAY) {
        lastLedBlinkTime = millis();
    }
}

void setup() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    motorsInit();
    sensorsInit();

#ifdef DEBUG_MOTORS
    Serial.begin(115200);
    while (!Serial) { ; }
    Serial.println("--- MODO DEBUG DE MOTORES HABILITADO ---");
    Serial.println("Envia comandos por Serial:");
    Serial.println("  f : Adelante (Forward)");
    Serial.println("  b : Atrás (Backward)");
    Serial.println("  l : Girar Izquierda (Left)");
    Serial.println("  r : Girar Derecha (Right)");
    Serial.println("  s : Frenar (Stop)");
#endif

    transitionTo(STATE_STANDBY);
}

void loop() {
#ifdef DEBUG_MOTORS
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd != '\r' && cmd != '\n') {
            switch (cmd) {
                case 'f':
                case 'F':
                    Serial.println("Comando: Adelante");
                    motorsSetSpeed(200, 200);
                    break;
                case 'b':
                case 'B':
                    Serial.println("Comando: Atrás");
                    motorsSetSpeed(-200, -200);
                    break;
                case 'l':
                case 'L':
                    Serial.println("Comando: Izquierda");
                    motorsSetSpeed(-200, 200);
                    break;
                case 'r':
                case 'R':
                    Serial.println("Comando: Derecha");
                    motorsSetSpeed(200, -200);
                    break;
                case 's':
                case 'S':
                    Serial.println("Comando: Frenar");
                    motorsBrake();
                    break;
                default:
                    Serial.print("Comando no reconocido: ");
                    Serial.println(cmd);
                    break;
            }
        }
    }
    return; // Evita ejecutar el bucle de la máquina de estados de combate
#endif

    // Priority 1: Check Line Sensors (only if we are active in combat)
    if (currentState != STATE_STANDBY && currentState != STATE_SAFETY_DELAY) {
        if (lineReadLeft() < LINE_THRESHOLD && currentState != STATE_EVADE_LEFT && currentState != STATE_EVADE_RIGHT) {
            transitionTo(STATE_EVADE_LEFT);
        } else if (lineReadRight() < LINE_THRESHOLD && currentState != STATE_EVADE_RIGHT && currentState != STATE_EVADE_LEFT) {
            transitionTo(STATE_EVADE_RIGHT);
        }
    }

    // Priority 2: Execute State Behaviors
    switch (currentState) {
        case STATE_STANDBY:
            motorsBrake();
            digitalWrite(PIN_LED, LOW);
            if (buttonPressed()) {
                // Wait for debounce and button release
                delay(150);
                while (buttonPressed()) { /* wait */ }
                transitionTo(STATE_SAFETY_DELAY);
            }
            break;

        case STATE_SAFETY_DELAY:
            motorsBrake();
            // Blink LED at 5Hz (every 100ms)
            if (millis() - lastLedBlinkTime >= LED_BLINK_INTERVAL) {
                lastLedBlinkTime = millis();
                ledState = !ledState;
                digitalWrite(PIN_LED, ledState ? HIGH : LOW);
            }

            // After 5 seconds, start the match
            if (millis() - stateStartTime >= SAFETY_DELAY_DURATION) {
                digitalWrite(PIN_LED, HIGH); // Steady ON when active
                transitionTo(STATE_INITIAL_TACTIC);
            }
            break;

        case STATE_INITIAL_TACTIC:
            // Spin clockwise looking for opponent
            motorsSetSpeed(INITIAL_TACTIC_SPEED, -INITIAL_TACTIC_SPEED);

            // Transition if opponent found
            {
                unsigned int dist = distanceRead();
                if (dist > 0 && dist < ATTACK_DISTANCE) {
                    transitionTo(STATE_ATTACK);
                    break;
                }
            }

            // Transition after timeout to standard search
            if (millis() - stateStartTime >= INITIAL_TACTIC_DURATION) {
                transitionTo(STATE_SEARCH);
            }
            break;

        case STATE_SEARCH:
            // Fast arc search sweep
            motorsSetSpeed(SEARCH_SPEED, -SEARCH_SPEED);

            // Check if opponent detected
            {
                unsigned int dist = distanceRead();
                if (dist > 0 && dist < ATTACK_DISTANCE) {
                    transitionTo(STATE_ATTACK);
                }
            }
            break;

        case STATE_ATTACK:
            // Charge straight at 100% speed
            motorsSetSpeed(ATTACK_SPEED, ATTACK_SPEED);

            // Check if opponent escaped
            {
                unsigned int dist = distanceRead();
                if (dist == 0 || dist >= ATTACK_DISTANCE) {
                    transitionTo(STATE_SEARCH);
                }
            }
            break;

        case STATE_EVADE_LEFT:
            // White line on the left. Backup and turn right.
            if (millis() - stateStartTime < EVADE_BACKUP_DURATION) {
                // Backup
                motorsSetSpeed(EVADE_BACKUP_SPEED, EVADE_BACKUP_SPEED);
            } else if (millis() - stateStartTime < EVADE_TOTAL_DURATION) {
                // Spin Right
                motorsSetSpeed(EVADE_SPIN_SPEED, -EVADE_SPIN_SPEED);
            } else {
                transitionTo(STATE_SEARCH);
            }
            break;

        case STATE_EVADE_RIGHT:
            // White line on the right. Backup and turn left.
            if (millis() - stateStartTime < EVADE_BACKUP_DURATION) {
                // Backup
                motorsSetSpeed(EVADE_BACKUP_SPEED, EVADE_BACKUP_SPEED);
            } else if (millis() - stateStartTime < EVADE_TOTAL_DURATION) {
                // Spin Left
                motorsSetSpeed(-EVADE_SPIN_SPEED, EVADE_SPIN_SPEED);
            } else {
                transitionTo(STATE_SEARCH);
            }
            break;
    }
}
