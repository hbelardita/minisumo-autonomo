#ifndef DOMAIN_ENGINE_H
#define DOMAIN_ENGINE_H

/**
 * @brief States of the finite state machine (FSM) of the Minisumo robot.
 */
enum class State {
    STANDBY,       // Initial standby state, motors off
    COUNTDOWN,     // Safety countdown with LED blinking
    OPENING_MOVE,  // Initial tactical move when combat starts
    SEARCH,        // Search for the opponent by spinning on its axis
    CHARGE,        // Direct attack/charge towards the detected opponent
    RECOVERY       // Evasion of the white line (backup and spin)
};

/**
 * @brief Operation modes of the motor driver.
 */
enum class MotorMode {
    STANDBY,       // Motors deactivated/off (STBY in LOW)
    BRAKE,         // Active braking (motors shorted)
    DRIVE          // Active speed control
};

/**
 * @brief Adjustable configuration parameters of the FSM.
 */
struct DomainConfig {
    unsigned long safetyMs = 5000;      // Safety countdown duration (SAFETY_MS)
    unsigned long tacticMs = 400;       // Duration of the initial tactical spin (TACTIC_MS)
    unsigned long backupMs = 250;       // Backup time during evasion (BACKUP_MS)
    unsigned long recoveryMs = 450;     // Total evasion/recovery time (EVADE_MS)
    unsigned long persistMs = 200;      // Attack persistence time without seeing the opponent (PERSIST_MS)
    unsigned long blinkMs = 100;        // LED blinking semiperiod at 5Hz (BLINK_MS)

    int backupSpeed = -180;             // Backup speed in evasion (EVADE_BACKUP_SPEED)
    int spinSpeed = 180;                // Spin speed in evasion (EVADE_SPIN_SPEED)
    int tacticSpeed = 160;              // Initial tactical spin speed (INITIAL_TACTIC_SPEED)
    int searchSpeed = 120;              // Search speed (SEARCH_SPEED)
    int chargeSpeed = 255;              // Attack/charge speed (ATTACK_SPEED)
};

/**
 * @brief System inputs from sensors and physical timers.
 */
struct DomainInputs {
    unsigned long currentTimeMs;        // Current time in milliseconds
    bool buttonPressed;                 // Start button state (true = pressed)
    bool opponentDetected;              // Opponent sensor state (true = detected)
    bool leftBorderDetected;            // Left line sensor (true = on white line)
    bool rightBorderDetected;           // Right line sensor (true = on white line)
};

/**
 * @brief System outputs for motors and actuators.
 */
struct DomainOutputs {
    State state;                        // Current state of the FSM
    MotorMode motorMode;                // Motor mode
    int leftMotorSpeed;                 // Left motor speed
    int rightMotorSpeed;                // Right motor speed
    bool ledOn;                         // LED state
};

/**
 * @brief Main controller of the FSM logic (DomainEngine).
 */
class DomainEngine {
public:
    /**
     * @brief Constructor with optional configuration.
     * @param config Structure with state machine parameters.
     */
    explicit DomainEngine(const DomainConfig& config = DomainConfig());

    /**
     * @brief Executes an FSM update cycle based on the current inputs.
     * @param inputs Current sensor and time inputs.
     * @return Desired outputs for motors and LED.
     */
    DomainOutputs update(const DomainInputs& inputs);

    /**
     * @brief Gets the current state of the machine.
     * @return Current state.
     */
    State getState() const { return currentState; }

private:
    DomainConfig config;                // Time and speed configuration
    State currentState;                 // Current internal state of the FSM
    unsigned long stateStartTimeMs;     // Timestamp of the last state transition
    
    // Control variables for charge persistence
    unsigned long timeLostMs;

    // Spin direction in the recovery state
    bool recoverySpinLeft;              // true = spin left (counter-clockwise), false = spin right (clockwise)

    /**
     * @brief Transitions to a new state and updates the state timer.
     * @param newState New target state.
     * @param currentTimeMs Timestamp of the event.
     */
    void transitionTo(State newState, unsigned long currentTimeMs);
};

#endif // DOMAIN_ENGINE_H
