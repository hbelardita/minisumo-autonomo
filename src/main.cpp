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
    STATE_SEARCH,
    STATE_APPROACH,
    STATE_ATTACK,
    STATE_EVADE_BACKUP,
    STATE_EVADE_TURN
};

State currentState = STATE_STANDBY;
unsigned long stateStartTime = 0;

// Sentido de búsqueda (girar a la derecha o a la izquierda)
bool searchClockwise = true;

// Dirección del giro de evasión (horario si tocó sensor izquierdo o ambos, antihorario si tocó derecho)
bool evadeTurnClockwise = true;

// Temporizador para persistencia de ataque (global de archivo)
static unsigned long timeLost = 0;

// Última distancia conocida durante aproximación (promovido a ámbito de archivo para persistencia y reseteo entre estados)
static unsigned int lastKnownDist = 0;

// Realiza la transición de estado y registra el tiempo de inicio
void transitionTo(State newState) {
    // Si entramos a buscar tras perder al oponente en combate (aproximación o ataque),
    // alternamos el sentido del giro respecto a la última vez (no al inicio de partida ni tras evasión)
    if (newState == STATE_SEARCH && (currentState == STATE_APPROACH || currentState == STATE_ATTACK)) {
        searchClockwise = !searchClockwise;
    }

    currentState = newState;
    stateStartTime = millis();

    // Se limpia el temporizador de pérdida y distancia conocida al cambiar de estado
    timeLost = 0;
    lastKnownDist = 0;

    // Inicializa la velocidad en aproximación para no arrastrar el giro residual de búsqueda
    if (newState == STATE_APPROACH) {
        motorsSetSpeed(APPROACH_SPEED, APPROACH_SPEED);
    }
}

// Chequeo de emergencia de línea post-distanceRead
// Devuelve true si detectó borde y ya transicionó a evasión
static bool checkLineEmergency() {
    bool leftRaw = lineReadLeftRaw();
    bool rightRaw = lineReadRightRaw();

    if (leftRaw || rightRaw) {
        if (leftRaw && rightRaw) {
            evadeTurnClockwise = true;  // Ambos sensores: horario por defecto
        } else if (leftRaw) {
            evadeTurnClockwise = true;  // Sensor izquierdo: giro horario
        } else {
            evadeTurnClockwise = false; // Sensor derecho: giro antihorario
        }
        transitionTo(STATE_EVADE_BACKUP);
        return true;
    }
    return false;
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

#ifdef DEBUG_SENSORS
    Serial.begin(9600);
    while (!Serial) { ; }
    Serial.println("--- MODO DEBUG DE SENSORES HABILITADO ---");
    Serial.println("Lecturas en tiempo real de TCRT5000 cada 200ms:");
#endif

    transitionTo(STATE_STANDBY);
}

void loop() {
#ifdef DEBUG_SENSORS
    static unsigned long lastSensorDebugPrint = 0;
    if (millis() - lastSensorDebugPrint >= 200) {
        lastSensorDebugPrint = millis();
        Serial.print("L_Analog: "); Serial.print(lineReadLeftAnalog());
        Serial.print(" | R_Analog: "); Serial.print(lineReadRightAnalog());
        Serial.print(" | L_Line: "); Serial.print(lineReadLeftRaw() ? "BLANCO" : "NEGRO");
        Serial.print(" | R_Line: "); Serial.println(lineReadRightRaw() ? "BLANCO" : "NEGRO");
    }
    return; // Evita el bucle de la máquina de estados si estamos en debug
#endif

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

    // Prioridad 1: Detección de sensores de línea blanca (solo activos en combate y fuera de evasión)
    if (currentState != STATE_STANDBY && currentState != STATE_SAFETY_DELAY &&
        currentState != STATE_EVADE_BACKUP && currentState != STATE_EVADE_TURN) {
        // Leemos siempre ambos para no romper el debounce estático
        bool leftLine = lineReadLeft();
        bool rightLine = lineReadRight();

        if (leftLine || rightLine) {
            if (leftLine && rightLine) {
                evadeTurnClockwise = true;  // Ambos sensores: horario por defecto
            } else if (leftLine) {
                evadeTurnClockwise = true;  // Sensor izquierdo: giro horario
            } else {
                evadeTurnClockwise = false; // Sensor derecho: giro antihorario
            }
            transitionTo(STATE_EVADE_BACKUP);
        }
    }

    // Prioridad 2: Comportamientos de cada estado
    switch (currentState) {
        case STATE_STANDBY:
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

            // Al cumplirse los 5 segundos, inicia el combate pasando directo a búsqueda
            if (millis() - stateStartTime >= SAFETY_MS) {
                digitalWrite(PIN_LED, HIGH); // LED encendido fijo durante combate
                transitionTo(STATE_SEARCH);
            }
            break;

        case STATE_SEARCH:
            // Giro sobre su propio eje a velocidad SEARCH_SPEED (100)
            if (searchClockwise) {
                motorsSetSpeed(SEARCH_SPEED, -SEARCH_SPEED);
            } else {
                motorsSetSpeed(-SEARCH_SPEED, SEARCH_SPEED);
            }

            // Transiciona si detecta oponente
            {
                unsigned int dist = distanceRead();
                if (checkLineEmergency()) break;
                if (dist > 0 && dist <= ATTACK_DISTANCE) {
                    transitionTo(STATE_ATTACK);
                } else if (dist > 0 && dist <= APPROACH_DISTANCE) {
                    transitionTo(STATE_APPROACH);
                }
            }
            break;

        case STATE_APPROACH:
            // Comprobación de distancia con el oponente y aceleración progresiva sin retardos
            {
                unsigned int dist = distanceRead();
                if (checkLineEmergency()) break;

                if (dist > 0 && dist <= ATTACK_DISTANCE) {
                    lastKnownDist = dist;
                    transitionTo(STATE_ATTACK);
                } else if (dist == 0 && lastKnownDist > 0 && lastKnownDist <= 18) {
                    // Zona ciega tras aproximación cercana (< 2 cm tras estar a <= 18 cm)
                    lastKnownDist = 0;
                    transitionTo(STATE_ATTACK);
                } else if (dist > 0 && dist <= 18) {
                    lastKnownDist = dist;
                    timeLost = 0;
                    motorsSetSpeed(RAMP_SPEED_18, RAMP_SPEED_18);
                } else if (dist > 0 && dist <= 25) {
                    lastKnownDist = dist;
                    timeLost = 0;
                    motorsSetSpeed(RAMP_SPEED_25, RAMP_SPEED_25);
                } else if (dist > 0 && dist <= 30) {
                    lastKnownDist = dist;
                    timeLost = 0;
                    motorsSetSpeed(RAMP_SPEED_30, RAMP_SPEED_30);
                } else if (dist > 0 && dist <= APPROACH_DISTANCE) {
                    lastKnownDist = dist;
                    timeLost = 0;
                    motorsSetSpeed(APPROACH_SPEED, APPROACH_SPEED);
                } else {
                    // Contacto perdido (dist == 0 sin contacto previo cercano, o dist > APPROACH_DISTANCE)
                    // Mantiene velocidad de aproximación frontal mientras dura el tiempo de persistencia
                    motorsSetSpeed(APPROACH_SPEED, APPROACH_SPEED);
                    lastKnownDist = 0;
                    if (timeLost == 0) {
                        timeLost = millis();
                    } else if (millis() - timeLost > PERSIST_MS) {
                        transitionTo(STATE_SEARCH);
                    }
                }
            }
            break;

        case STATE_ATTACK:
            // Carga directa al 100% de velocidad
            motorsSetSpeed(ATTACK_SPEED, ATTACK_SPEED);

            // Comprobación de escape del oponente
            {
                unsigned int dist = distanceRead();
                if (checkLineEmergency()) break;

                if (dist > 0 && dist < ATTACK_RELEASE_DISTANCE) {
                    timeLost = 0; // Contacto confirmado en rango de ataque
                } else if (dist >= ATTACK_RELEASE_DISTANCE && dist <= APPROACH_DISTANCE) {
                    // El oponente logró separarse: pasamos a aproximación con histéresis
                    transitionTo(STATE_APPROACH);
                } else {
                    // dist == 0 (posible zona ciega < 2 cm o rival esquivó) o dist > APPROACH_DISTANCE (fuera de alcance).
                    // Se mantiene la carga por persistencia; si no hay contacto o línea tras PERSIST_MS,
                    // se asume escape y se vuelve a buscar.
                    if (timeLost == 0) {
                        timeLost = millis(); // Inicia cuenta de persistencia
                    } else if (millis() - timeLost > PERSIST_MS) {
                        // Vuelve a buscar si pasa el tiempo de persistencia
                        transitionTo(STATE_SEARCH);
                    }
                }
            }
            break;

        case STATE_EVADE_BACKUP:
            // Fase 1: micro-retroceso en línea recta para despejar la pala
            if (millis() - stateStartTime < EVADE_BACKUP_MS) {
                motorsSetSpeed(EVADE_BACKUP_SPEED_L, EVADE_BACKUP_SPEED_R);
            } else {
                transitionTo(STATE_EVADE_TURN);
            }
            break;

        case STATE_EVADE_TURN:
            // Fase 2: giro puro sobre su eje calibrado a 90°-100°
            if (millis() - stateStartTime < EVADE_TURN_MS) {
                if (evadeTurnClockwise) {
                    // Giro horario: rueda izquierda adelante, derecha atrás
                    motorsSetSpeed(EVADE_SPIN_SPEED, -EVADE_SPIN_SPEED);
                } else {
                    // Giro antihorario: rueda izquierda atrás, derecha adelante
                    motorsSetSpeed(-EVADE_SPIN_SPEED, EVADE_SPIN_SPEED);
                }
            } else {
                // Al completar el giro, sincronizar searchClockwise y transicionar a búsqueda
                searchClockwise = evadeTurnClockwise;
                transitionTo(STATE_SEARCH);
            }
            break;
    }
}
