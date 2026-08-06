#include <Arduino.h>
#include "config.h"
#include "motors.h"
#include "sensors.h"
#include "domain_engine.h"

// Physical pin mapping (defined in config.h)
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

DomainConfig buildDomainConfig() {
    DomainConfig cfg;
    cfg.safetyMs = SAFETY_MS;
    cfg.tacticMs = TACTIC_MS;
    cfg.backupMs = BACKUP_MS;
    cfg.recoveryMs = EVADE_MS; // total evade time
    cfg.persistMs = PERSIST_MS;
    cfg.blinkMs = BLINK_MS;
    
    cfg.backupSpeed = EVADE_BACKUP_SPEED;
    cfg.spinSpeed = EVADE_SPIN_SPEED;
    cfg.tacticSpeed = INITIAL_TACTIC_SPEED;
    cfg.searchSpeed = SEARCH_SPEED;
    cfg.chargeSpeed = ATTACK_SPEED;
    return cfg;
}

DomainEngine domainEngine(buildDomainConfig());

void setup() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    motorsInit();
    sensorsInit();

#ifdef DEBUG_MOTORS
    Serial.begin(115200);
    while (!Serial) { ; }
    Serial.println("--- MOTOR DEBUG MODE ENABLED ---");
    Serial.println("Send commands via Serial:");
    Serial.println("  f : Forward");
    Serial.println("  b : Backward");
    Serial.println("  l : Turn Left");
    Serial.println("  r : Turn Right");
    Serial.println("  s : Stop");
#endif
}

void loop() {
#ifdef DEBUG_MOTORS
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd != '\r' && cmd != '\n') {
            switch (cmd) {
                case 'f':
                case 'F':
                    Serial.println("Command: Forward");
                    motorsSetSpeed(200, 200);
                    break;
                case 'b':
                case 'B':
                    Serial.println("Command: Backward");
                    motorsSetSpeed(-200, -200);
                    break;
                case 'l':
                case 'L':
                    Serial.println("Command: Left");
                    motorsSetSpeed(-200, 200);
                    break;
                case 'r':
                case 'R':
                    Serial.println("Command: Right");
                    motorsSetSpeed(200, -200);
                    break;
                case 's':
                case 'S':
                    Serial.println("Command: Stop");
                    motorsBrake();
                    break;
                default:
                    Serial.print("Command not recognized: ");
                    Serial.println(cmd);
                    break;
            }
        }
    }
    return; // Avoids the state machine loop if in debug mode
#endif

    DomainInputs inputs;
    inputs.currentTimeMs = millis();
    inputs.buttonPressed = false;
    inputs.opponentDetected = false;
    inputs.leftBorderDetected = false;
    inputs.rightBorderDetected = false;

    State currentEngineState = domainEngine.getState();

    // Debouncing and button release in STANDBY state
    if (currentEngineState == State::STANDBY && buttonPressed()) {
        delay(150);
        while (buttonPressed()) {
            // Wait for the button to be released
        }
        inputs.buttonPressed = true;
    }

    // Optimized reading of the distance sensor
    if (currentEngineState == State::OPENING_MOVE || 
        currentEngineState == State::SEARCH || 
        currentEngineState == State::CHARGE) {
        unsigned int dist = distanceRead();
        inputs.opponentDetected = (dist > 0 && dist < ATTACK_DISTANCE);
    }

    // Reading of the line sensors (inactive in STANDBY)
    if (currentEngineState != State::STANDBY) {
        inputs.leftBorderDetected = lineReadLeft();
        inputs.rightBorderDetected = lineReadRight();
    }

    // Updates the FSM and obtains the desired outputs
    DomainOutputs outputs = domainEngine.update(inputs);

    // Applies the outputs to the motors
    if (outputs.motorMode == MotorMode::STANDBY) {
        motorsStandby();
    } else if (outputs.motorMode == MotorMode::BRAKE) {
        motorsBrake();
    } else if (outputs.motorMode == MotorMode::DRIVE) {
        motorsSetSpeed(outputs.leftMotorSpeed, outputs.rightMotorSpeed);
    }

    // Applies the outputs to the indicator LED
    digitalWrite(PIN_LED, outputs.ledOn ? HIGH : LOW);
}
