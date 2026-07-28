#include <Arduino.h>
#include "config.h"
#include "motors.h"
#include "sensors.h"

// Mapeo físico de pines (definidos en config.h)
const uint8_t PIN_STBY = 8;
const uint8_t PIN_AIN1 = 7;
const uint8_t PIN_AIN2 = 6;
const uint8_t PIN_PWMA = 5;
const uint8_t PIN_BIN1 = 4;
const uint8_t PIN_BIN2 = 2;
const uint8_t PIN_PWMB = 9;

const uint8_t PIN_TRIG = 11;
const uint8_t PIN_ECHO = 12;

const uint8_t PIN_TCRT_LEFT = A2;
const uint8_t PIN_TCRT_RIGHT = A3;

const uint8_t PIN_BUTTON = 10;
const uint8_t PIN_LED = 13;

// Estados de la máquina de estados finitos (FSM)
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

// Temporizador para persistencia de ataque (corregido de static local a global de archivo)
static unsigned long timeLost = 0;

// Realiza la transición de estado y registra el tiempo de inicio
void transitionTo(State newState) {
    currentState = newState;
    stateStartTime = millis();

    // CORRECCIÓN: Se limpia el temporizador de pérdida al entrar en estado de ataque
    if (newState == STATE_ATTACK) {
        timeLost = 0;
    }
}

void setup() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    motorsInit();
    sensorsInit();

#ifdef DEBUG_MOTORS
    Serial.begin(9600);
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
            Serial.print("> Recibido: ");
            Serial.println(cmd);
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
    return; // Evita el bucle de la máquina de estados si estamos en debug
#endif

    // Prioridad 1: Detección de sensores de línea blanca (solo activos en combate)
    if (currentState != STATE_STANDBY && currentState != STATE_SAFETY_DELAY) {
        // Leemos siempre ambos para no romper el debounce estático
        bool leftLine = lineReadLeft();
        bool rightLine = lineReadRight();

        if (leftLine && currentState != STATE_EVADE_LEFT && currentState != STATE_EVADE_RIGHT) {
            transitionTo(STATE_EVADE_LEFT);
        } else if (rightLine && currentState != STATE_EVADE_RIGHT && currentState != STATE_EVADE_LEFT) {
            transitionTo(STATE_EVADE_RIGHT);
        }
    }

    // Prioridad 2: Comportamientos de cada estado
    switch (currentState) {
        case STATE_STANDBY:
            // CORRECCIÓN: Se apaga el driver (Standby) en vez de frenar activamente
            motorsStandby();
            digitalWrite(PIN_LED, LOW);
            if (buttonPressed()) {
                // Espera antirrebote y liberación de botón
                delay(150);
                while (buttonPressed()) { /* esperar */ }
                transitionTo(STATE_SAFETY_DELAY);
            }
            break;

        case STATE_SAFETY_DELAY:
            motorsBrake();
            // Parpadea el LED a 5Hz durante la cuenta regresiva
            digitalWrite(PIN_LED, ((millis() - stateStartTime) / BLINK_MS) % 2 ? HIGH : LOW);

            // Al cumplirse los 5 segundos, inicia el combate
            if (millis() - stateStartTime >= SAFETY_MS) {
                digitalWrite(PIN_LED, HIGH); // LED encendido fijo durante combate
                transitionTo(STATE_INITIAL_TACTIC);
            }
            break;

        case STATE_INITIAL_TACTIC:
            // Giro inicial horario a velocidad táctica
            motorsSetSpeed(INITIAL_TACTIC_SPEED, -INITIAL_TACTIC_SPEED);

            // Transiciona si detecta oponente en frente
            {
                unsigned int dist = distanceRead();
                if (dist > 0 && dist < ATTACK_DISTANCE) {
                    transitionTo(STATE_ATTACK);
                    break;
                }
            }

            // Pasa a búsqueda estándar si se agota el tiempo de la táctica
            if (millis() - stateStartTime >= TACTIC_MS) {
                transitionTo(STATE_SEARCH);
            }
            break;

        case STATE_SEARCH:
            // Giro de búsqueda constante sobre el eje
            motorsSetSpeed(SEARCH_SPEED, -SEARCH_SPEED);

            // Transiciona si detecta oponente
            {
                unsigned int dist = distanceRead();
                if (dist > 0 && dist < ATTACK_DISTANCE) {
                    transitionTo(STATE_ATTACK);
                }
            }
            break;

        case STATE_ATTACK:
            // Carga directa al 100% de velocidad
            motorsSetSpeed(ATTACK_SPEED, ATTACK_SPEED);

            // Comprobación de escape del oponente
            {
                unsigned int dist = distanceRead();

                if (dist > 0 && dist < ATTACK_DISTANCE) {
                    timeLost = 0; // Vemos al rival claro, reseteamos contador de pérdida
                } else {
                    // Si perdemos contacto visual
                    if (timeLost == 0) {
                        timeLost = millis(); // Inicia cuenta de persistencia
                    } else if (millis() - timeLost > PERSIST_MS) {
                        // Vuelve a buscar si pasa el tiempo de persistencia
                        transitionTo(STATE_SEARCH);
                        timeLost = 0;
                    }
                }
            }
            break;

        case STATE_EVADE_LEFT:
            // Línea blanca detectada en la izquierda: retrocede y gira a la derecha
            if (millis() - stateStartTime < BACKUP_MS) {
                motorsSetSpeed(EVADE_BACKUP_SPEED, EVADE_BACKUP_SPEED);
            } else if (millis() - stateStartTime < EVADE_MS) {
                motorsSetSpeed(EVADE_SPIN_SPEED, -EVADE_SPIN_SPEED);
            } else {
                transitionTo(STATE_SEARCH);
            }
            break;

        case STATE_EVADE_RIGHT:
            // Línea blanca detectada en la derecha: retrocede y gira a la izquierda
            if (millis() - stateStartTime < BACKUP_MS) {
                motorsSetSpeed(EVADE_BACKUP_SPEED, EVADE_BACKUP_SPEED);
            } else if (millis() - stateStartTime < EVADE_MS) {
                motorsSetSpeed(-EVADE_SPIN_SPEED, EVADE_SPIN_SPEED);
            } else {
                transitionTo(STATE_SEARCH);
            }
            break;
    }
}
