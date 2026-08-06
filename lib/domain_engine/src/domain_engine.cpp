#include "domain_engine.h"

DomainEngine::DomainEngine(const DomainConfig& config)
    : config(config),
      currentState(State::STANDBY),
      stateStartTimeMs(0),
      timeLostMs(0),
      recoverySpinLeft(false) {}

void DomainEngine::transitionTo(State newState, unsigned long currentTimeMs) {
    currentState = newState;
    stateStartTimeMs = currentTimeMs;
    if (newState == State::CHARGE) {
        timeLostMs = 0;
    }
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
            // Priority 1: Border detection during countdown
            if (inputs.leftBorderDetected || inputs.rightBorderDetected) {
                if (inputs.leftBorderDetected) {
                    recoverySpinLeft = false; // Clockwise spin (spin right) by default if both are detected
                } else {
                    recoverySpinLeft = true;  // Counter-clockwise spin (spin left)
                }
                transitionTo(State::RECOVERY, inputs.currentTimeMs);
            }
            // Priority 2: Safety time expiration
            else if (inputs.currentTimeMs - stateStartTimeMs >= config.safetyMs) {
                transitionTo(State::OPENING_MOVE, inputs.currentTimeMs);
            }
            break;
        }
        case State::OPENING_MOVE: {
            // Priority 1: Border detection
            if (inputs.leftBorderDetected || inputs.rightBorderDetected) {
                if (inputs.leftBorderDetected) {
                    recoverySpinLeft = false;
                } else {
                    recoverySpinLeft = true;
                }
                transitionTo(State::RECOVERY, inputs.currentTimeMs);
            }
            // Priority 2: Opponent detection
            else if (inputs.opponentDetected) {
                transitionTo(State::CHARGE, inputs.currentTimeMs);
            }
            // Priority 3: Initial tactical time expiration
            else if (inputs.currentTimeMs - stateStartTimeMs >= config.tacticMs) {
                transitionTo(State::SEARCH, inputs.currentTimeMs);
            }
            break;
        }
        case State::SEARCH: {
            // Priority 1: Border detection
            if (inputs.leftBorderDetected || inputs.rightBorderDetected) {
                if (inputs.leftBorderDetected) {
                    recoverySpinLeft = false;
                } else {
                    recoverySpinLeft = true;
                }
                transitionTo(State::RECOVERY, inputs.currentTimeMs);
            }
            // Priority 2: Opponent detection
            else if (inputs.opponentDetected) {
                transitionTo(State::CHARGE, inputs.currentTimeMs);
            }
            break;
        }
        case State::CHARGE: {
            // Priority 1: Border detection
            if (inputs.leftBorderDetected || inputs.rightBorderDetected) {
                if (inputs.leftBorderDetected) {
                    recoverySpinLeft = false;
                } else {
                    recoverySpinLeft = true;
                }
                transitionTo(State::RECOVERY, inputs.currentTimeMs);
            }
            // Priority 2: Attack behavior and persistence grace time
            else {
                if (inputs.opponentDetected) {
                    // The opponent is still in sight, reset the lost timer
                    timeLostMs = 0;
                } else {
                    // Opponent lost from sight, start or verify timer
                    if (timeLostMs == 0) {
                        timeLostMs = inputs.currentTimeMs;
                    } else if (inputs.currentTimeMs - timeLostMs >= config.persistMs) {
                        transitionTo(State::SEARCH, inputs.currentTimeMs);
                        timeLostMs = 0;
                    }
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
            outputs.leftMotorSpeed = config.tacticSpeed;
            outputs.rightMotorSpeed = -config.tacticSpeed;
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
