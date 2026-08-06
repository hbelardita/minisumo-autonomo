#include "domain_engine.h"

DomainEngine::DomainEngine(const DomainConfig& config)
    : config(config),
      currentState(State::STANDBY),
      stateStartTimeMs(0),
      lossDetectedAtMs(0),
      recoverySpinLeft(false) {}

void DomainEngine::transitionTo(State newState, unsigned long currentTimeMs) {
    currentState = newState;
    stateStartTimeMs = currentTimeMs;
    if (newState == State::CHARGE) {
        lossDetectedAtMs = 0;
    }
}

bool DomainEngine::checkBorderTransition(const DomainInputs& inputs) {
    if (inputs.leftBorderDetected || inputs.rightBorderDetected) {
        if (inputs.leftBorderDetected) {
            recoverySpinLeft = false;
        } else {
            recoverySpinLeft = true;
        }
        transitionTo(State::RECOVERY, inputs.currentTimeMs);
        return true;
    }
    return false;
}

DomainOutputs DomainEngine::update(const DomainInputs& inputs) {
    // 1. FSM state transitions handling
    switch (currentState) {
        case State::STANDBY: {
            if (inputs.buttonPressed) {
                transitionTo(State::COUNTDOWN, inputs.currentTimeMs);
            }
            break;
        }
        case State::COUNTDOWN: {
            // Countdown duration (COUNTDOWN_MS) expiration
            if (inputs.currentTimeMs - stateStartTimeMs >= config.countdownMs) {
                transitionTo(State::OPENING_MOVE, inputs.currentTimeMs);
            }
            break;
        }
        case State::OPENING_MOVE: {
            // Priority 1: Border detection
            if (checkBorderTransition(inputs)) {
                break;
            }
            // Priority 2: Opponent detection
            if (inputs.opponentDetected) {
                transitionTo(State::CHARGE, inputs.currentTimeMs);
            }
            // Priority 3: Initial opening move time expiration
            else if (inputs.currentTimeMs - stateStartTimeMs >= config.openingMoveMs) {
                transitionTo(State::SEARCH, inputs.currentTimeMs);
            }
            break;
        }
        case State::SEARCH: {
            // Priority 1: Border detection
            if (checkBorderTransition(inputs)) {
                break;
            }
            // Priority 2: Opponent detection
            if (inputs.opponentDetected) {
                transitionTo(State::CHARGE, inputs.currentTimeMs);
            }
            break;
        }
        case State::CHARGE: {
            // Priority 1: Border detection
            if (checkBorderTransition(inputs)) {
                break;
            }
            // Priority 2: Charge behavior and persistence grace time
            if (inputs.opponentDetected) {
                // The opponent is still in sight, reset the lost timer
                lossDetectedAtMs = 0;
            } else {
                // Opponent lost from sight, start or verify timer
                if (lossDetectedAtMs == 0) {
                    lossDetectedAtMs = inputs.currentTimeMs;
                } else if (inputs.currentTimeMs - lossDetectedAtMs >= config.persistMs) {
                    transitionTo(State::SEARCH, inputs.currentTimeMs);
                    lossDetectedAtMs = 0;
                }
            }
            break;
        }
        case State::RECOVERY: {
            // The recovery state is atomic and runs to completion (recoveryMs)
            // It ignores opponent detections and new line detections
            if (inputs.currentTimeMs - stateStartTimeMs >= config.recoveryMs) {
                transitionTo(State::SEARCH, inputs.currentTimeMs);
            }
            break;
        }
    }

    // 2. Generation of system outputs based on the current state
    DomainOutputs outputs;
    outputs.state = currentState;

    switch (currentState) {
        case State::STANDBY: {
            outputs.motorMode = MotorMode::STANDBY;
            outputs.leftMotorSpeed = 0;
            outputs.rightMotorSpeed = 0;
            outputs.ledOn = false;
            break;
        }
        case State::COUNTDOWN: {
            outputs.motorMode = MotorMode::BRAKE;
            outputs.leftMotorSpeed = 0;
            outputs.rightMotorSpeed = 0;
            unsigned long elapsed = inputs.currentTimeMs - stateStartTimeMs;
            outputs.ledOn = ((elapsed / config.blinkMs) % 2) != 0;
            break;
        }
        case State::OPENING_MOVE: {
            outputs.motorMode = MotorMode::DRIVE;
            outputs.leftMotorSpeed = config.openingMoveSpeed;
            outputs.rightMotorSpeed = -config.openingMoveSpeed;
            outputs.ledOn = true;
            break;
        }
        case State::SEARCH: {
            outputs.motorMode = MotorMode::DRIVE;
            outputs.leftMotorSpeed = config.searchSpeed;
            outputs.rightMotorSpeed = -config.searchSpeed;
            outputs.ledOn = true;
            break;
        }
        case State::CHARGE: {
            outputs.motorMode = MotorMode::DRIVE;
            outputs.leftMotorSpeed = config.chargeSpeed;
            outputs.rightMotorSpeed = config.chargeSpeed;
            outputs.ledOn = true;
            break;
        }
        case State::RECOVERY: {
            outputs.motorMode = MotorMode::DRIVE;
            outputs.ledOn = true;
            unsigned long elapsed = inputs.currentTimeMs - stateStartTimeMs;

            if (elapsed < config.backupMs) {
                // Phase 1: Backup
                outputs.leftMotorSpeed = config.backupSpeed;
                outputs.rightMotorSpeed = config.backupSpeed;
            } else {
                // Phase 2: Spin depending on the sensor that triggered the state
                if (recoverySpinLeft) {
                    // Right border: spin to the left
                    outputs.leftMotorSpeed = -config.spinSpeed;
                    outputs.rightMotorSpeed = config.spinSpeed;
                } else {
                    // Left border or both: spin to the right
                    outputs.leftMotorSpeed = config.spinSpeed;
                    outputs.rightMotorSpeed = -config.spinSpeed;
                }
            }
            break;
        }
    }

    return outputs;
}
