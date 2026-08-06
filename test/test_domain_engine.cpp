#include <unity.h>
#include "domain_engine.h"

// Quick configuration for instant unit tests without real-time delays
DomainConfig createTestConfig() {
    DomainConfig cfg;
    cfg.safetyMs = 1000;
    cfg.tacticMs = 100;
    cfg.backupMs = 50;
    cfg.recoveryMs = 150;
    cfg.persistMs = 60;
    cfg.blinkMs = 20;

    cfg.backupSpeed = -180;
    cfg.spinSpeed = 180;
    cfg.tacticSpeed = 160;
    cfg.searchSpeed = 120;
    cfg.chargeSpeed = 255;
    return cfg;
}

void setUp(void) {
    // Initialization before each test (empty in this case)
}

void tearDown(void) {
    // Cleanup after each test (empty in this case)
}

// 1. Countdown duration: Verifies that the countdown lasts exactly as configured and flashes the LED
void test_countdown_duration() {
    DomainConfig cfg = createTestConfig();
    DomainEngine engine(cfg);
    DomainInputs inputs = {0, false, false, false, false};

    // Start in STANDBY
    TEST_ASSERT_EQUAL(State::STANDBY, engine.getState());

    // The button is pressed to start the countdown
    inputs.buttonPressed = true;
    inputs.currentTimeMs = 10;
    DomainOutputs outputs = engine.update(inputs);

    TEST_ASSERT_EQUAL(State::COUNTDOWN, engine.getState());
    TEST_ASSERT_EQUAL(MotorMode::BRAKE, outputs.motorMode);
    TEST_ASSERT_EQUAL(0, outputs.leftMotorSpeed);
    TEST_ASSERT_EQUAL(0, outputs.rightMotorSpeed);

    // With elapsed time less than safetyMs, it remains in COUNTDOWN
    inputs.buttonPressed = false;
    inputs.currentTimeMs = 500; // 490ms elapsed
    outputs = engine.update(inputs);

    TEST_ASSERT_EQUAL(State::COUNTDOWN, engine.getState());
    TEST_ASSERT_EQUAL(MotorMode::BRAKE, outputs.motorMode);

    // Flashing at 510ms (elapsed = 500ms, blinkMs = 20. 500 / 20 = 25 % 2 != 0 -> LED ON)
    inputs.currentTimeMs = 510;
    outputs = engine.update(inputs);
    TEST_ASSERT_TRUE(outputs.ledOn);

    // Flashing at 530ms (elapsed = 520ms, blinkMs = 20. 520 / 20 = 26 % 2 == 0 -> LED OFF)
    inputs.currentTimeMs = 530;
    outputs = engine.update(inputs);
    TEST_ASSERT_FALSE(outputs.ledOn);
}

// 2. Opening move starts after countdown: Verifies transition to tactical move when the countdown expires
void test_opening_move_starts_after_countdown() {
    DomainConfig cfg = createTestConfig();
    DomainEngine engine(cfg);
    DomainInputs inputs = {0, true, false, false, false};

    // Start countdown at t=0
    engine.update(inputs);
    TEST_ASSERT_EQUAL(State::COUNTDOWN, engine.getState());

    // Expire countdown (safetyMs = 1000)
    inputs.buttonPressed = false;
    inputs.currentTimeMs = 1000;
    DomainOutputs outputs = engine.update(inputs);

    // Must transition to OPENING_MOVE spinning right
    TEST_ASSERT_EQUAL(State::OPENING_MOVE, engine.getState());
    TEST_ASSERT_EQUAL(MotorMode::DRIVE, outputs.motorMode);
    TEST_ASSERT_EQUAL(cfg.tacticSpeed, outputs.leftMotorSpeed);
    TEST_ASSERT_EQUAL(-cfg.tacticSpeed, outputs.rightMotorSpeed);
    TEST_ASSERT_TRUE(outputs.ledOn);
}

// 3. Opening move interruptible by detection: Verifies that detecting an opponent interrupts the tactical move
void test_opening_move_interruptible_by_detection() {
    DomainConfig cfg = createTestConfig();
    DomainEngine engine(cfg);
    DomainInputs inputs = {0, true, false, false, false};

    // Start countdown and transition to OPENING_MOVE
    engine.update(inputs);
    inputs.buttonPressed = false;
    inputs.currentTimeMs = 1000;
    engine.update(inputs);
    TEST_ASSERT_EQUAL(State::OPENING_MOVE, engine.getState());

    // Detect opponent during OPENING_MOVE
    inputs.currentTimeMs = 1050;
    inputs.opponentDetected = true;
    DomainOutputs outputs = engine.update(inputs);

    // Must transition immediately to CHARGE at attack speed
    TEST_ASSERT_EQUAL(State::CHARGE, engine.getState());
    TEST_ASSERT_EQUAL(MotorMode::DRIVE, outputs.motorMode);
    TEST_ASSERT_EQUAL(cfg.chargeSpeed, outputs.leftMotorSpeed);
    TEST_ASSERT_EQUAL(cfg.chargeSpeed, outputs.rightMotorSpeed);
}

// 4. Opening move transitions to Search: Transitions to SEARCH if tacticMs expires
void test_opening_move_transitions_to_search() {
    DomainConfig cfg = createTestConfig();
    DomainEngine engine(cfg);
    DomainInputs inputs = {0, true, false, false, false};

    // Start countdown and transition to OPENING_MOVE
    engine.update(inputs);
    inputs.buttonPressed = false;
    inputs.currentTimeMs = 1000;
    engine.update(inputs);
    TEST_ASSERT_EQUAL(State::OPENING_MOVE, engine.getState());

    // Expire tactical time (tacticMs = 100ms, from t=1000ms to t=1100ms) without detections
    inputs.currentTimeMs = 1100;
    DomainOutputs outputs = engine.update(inputs);

    // Must transition to SEARCH spinning on its axis
    TEST_ASSERT_EQUAL(State::SEARCH, engine.getState());
    TEST_ASSERT_EQUAL(MotorMode::DRIVE, outputs.motorMode);
    TEST_ASSERT_EQUAL(cfg.searchSpeed, outputs.leftMotorSpeed);
    TEST_ASSERT_EQUAL(-cfg.searchSpeed, outputs.rightMotorSpeed);
}

// 5. Search transitions to Charge on detection: Detecting an opponent during search starts the charge
void test_search_transitions_to_charge_on_detection() {
    DomainConfig cfg = createTestConfig();
    DomainEngine engine(cfg);
    DomainInputs inputs = {0, true, false, false, false};

    // Position at SEARCH
    engine.update(inputs); // countdown at t=0
    inputs.buttonPressed = false;
    inputs.currentTimeMs = 1000;
    engine.update(inputs); // opening move at t=1000
    inputs.currentTimeMs = 1100;
    engine.update(inputs); // search at t=1100
    TEST_ASSERT_EQUAL(State::SEARCH, engine.getState());

    // Detect opponent during search
    inputs.currentTimeMs = 1150;
    inputs.opponentDetected = true;
    DomainOutputs outputs = engine.update(inputs);

    // Transitions to CHARGE
    TEST_ASSERT_EQUAL(State::CHARGE, engine.getState());
    TEST_ASSERT_EQUAL(cfg.chargeSpeed, outputs.leftMotorSpeed);
    TEST_ASSERT_EQUAL(cfg.chargeSpeed, outputs.rightMotorSpeed);
}

// 6. Charge persistence: Verifies the grace time when losing the opponent and its reset when re-detecting them
void test_charge_persistence() {
    DomainConfig cfg = createTestConfig();
    DomainEngine engine(cfg);
    DomainInputs inputs = {0, true, false, false, false};

    // Position at CHARGE
    engine.update(inputs); // countdown at t=0
    inputs.buttonPressed = false;
    inputs.currentTimeMs = 1000;
    engine.update(inputs); // opening move at t=1000
    inputs.currentTimeMs = 1050;
    inputs.opponentDetected = true;
    engine.update(inputs); // charge at t=1050
    TEST_ASSERT_EQUAL(State::CHARGE, engine.getState());

    // Lose opponent at t=1100
    inputs.opponentDetected = false;
    inputs.currentTimeMs = 1100;
    DomainOutputs outputs = engine.update(inputs);

    // Must persist in CHARGE during the grace time (persistMs = 60ms)
    TEST_ASSERT_EQUAL(State::CHARGE, engine.getState());

    // Advance 40ms after (t=1140). Still in grace period.
    inputs.currentTimeMs = 1140;
    outputs = engine.update(inputs);
    TEST_ASSERT_EQUAL(State::CHARGE, engine.getState());

    // Re-detect opponent at t=1150, resetting persistence
    inputs.opponentDetected = true;
    inputs.currentTimeMs = 1150;
    outputs = engine.update(inputs);
    TEST_ASSERT_EQUAL(State::CHARGE, engine.getState());

    // Lose opponent again at t=1160
    inputs.opponentDetected = false;
    inputs.currentTimeMs = 1160;
    outputs = engine.update(inputs);
    TEST_ASSERT_EQUAL(State::CHARGE, engine.getState());

    // Advance 50ms after (t=1210). Still in CHARGE
    inputs.currentTimeMs = 1210;
    outputs = engine.update(inputs);
    TEST_ASSERT_EQUAL(State::CHARGE, engine.getState());

    // Advance to t=1220 (60ms since lost), must expire and transition to SEARCH
    inputs.currentTimeMs = 1220;
    outputs = engine.update(inputs);
    TEST_ASSERT_EQUAL(State::SEARCH, engine.getState());
}

// 7. Recovery priority: Verify that border detection interrupts any combat state
void test_recovery_priority() {
    DomainConfig cfg = createTestConfig();
    
    // Interruption from Countdown
    {
        DomainEngine engine(cfg);
        DomainInputs inputs = {0, true, false, false, false};
        engine.update(inputs);
        TEST_ASSERT_EQUAL(State::COUNTDOWN, engine.getState());
        
        inputs.currentTimeMs = 100;
        inputs.leftBorderDetected = true;
        engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());
    }

    // Interruption from Opening Move
    {
        DomainEngine engine(cfg);
        DomainInputs inputs = {0, true, false, false, false};
        engine.update(inputs);
        inputs.buttonPressed = false;
        inputs.currentTimeMs = 1000;
        engine.update(inputs);
        TEST_ASSERT_EQUAL(State::OPENING_MOVE, engine.getState());

        inputs.currentTimeMs = 1050;
        inputs.rightBorderDetected = true;
        engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());
    }

    // Interruption from Search
    {
        DomainEngine engine(cfg);
        DomainInputs inputs = {0, true, false, false, false};
        engine.update(inputs);
        inputs.buttonPressed = false;
        inputs.currentTimeMs = 1000;
        engine.update(inputs);
        inputs.currentTimeMs = 1100;
        engine.update(inputs);
        TEST_ASSERT_EQUAL(State::SEARCH, engine.getState());

        inputs.currentTimeMs = 1200;
        inputs.leftBorderDetected = true;
        engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());
    }

    // Interruption from Charge
    {
        DomainEngine engine(cfg);
        DomainInputs inputs = {0, true, false, false, false};
        engine.update(inputs);
        inputs.buttonPressed = false;
        inputs.currentTimeMs = 1000;
        engine.update(inputs);
        inputs.currentTimeMs = 1050;
        inputs.opponentDetected = true;
        engine.update(inputs);
        TEST_ASSERT_EQUAL(State::CHARGE, engine.getState());

        inputs.currentTimeMs = 1080;
        inputs.rightBorderDetected = true;
        engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());
    }
}

// 8. Recovery atomicity: Evasion ignores other border or opponent signals until completed
void test_recovery_atomicity() {
    DomainConfig cfg = createTestConfig();
    DomainEngine engine(cfg);
    DomainInputs inputs = {0, true, false, false, false};

    // Go to SEARCH and trigger RECOVERY by left border at t=1200
    engine.update(inputs); // countdown
    inputs.buttonPressed = false;
    inputs.currentTimeMs = 1000;
    engine.update(inputs); // opening move
    inputs.currentTimeMs = 1100;
    engine.update(inputs); // search
    
    inputs.currentTimeMs = 1200;
    inputs.leftBorderDetected = true;
    DomainOutputs outputs = engine.update(inputs);
    TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());
    
    // At t=1230 (Phase 1: backup), opponent and right border are detected simultaneously
    inputs.leftBorderDetected = false;
    inputs.opponentDetected = true;
    inputs.rightBorderDetected = true;
    inputs.currentTimeMs = 1230;
    outputs = engine.update(inputs);
    
    // Must continue in RECOVERY and keep backing up
    TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());
    TEST_ASSERT_EQUAL(cfg.backupSpeed, outputs.leftMotorSpeed);
    TEST_ASSERT_EQUAL(cfg.backupSpeed, outputs.rightMotorSpeed);
}

// 9. Recovery transitions to Search after completion: Evasion divided into phases and return to search
void test_recovery_transitions_to_search_after_completion() {
    DomainConfig cfg = createTestConfig();
    
    // Test 2a: Triggered by left border -> spins right
    {
        DomainEngine engine(cfg);
        DomainInputs inputs = {0, true, false, false, false};
        engine.update(inputs); // countdown
        inputs.buttonPressed = false;
        inputs.currentTimeMs = 1000;
        engine.update(inputs); // opening move
        inputs.currentTimeMs = 1100;
        engine.update(inputs); // search

        // Left border at t=1200
        inputs.currentTimeMs = 1200;
        inputs.leftBorderDetected = true;
        DomainOutputs outputs = engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());

        // Phase 1: t=1240 (< 50ms) -> backup
        inputs.leftBorderDetected = false;
        inputs.currentTimeMs = 1240;
        outputs = engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());
        TEST_ASSERT_EQUAL(cfg.backupSpeed, outputs.leftMotorSpeed);
        TEST_ASSERT_EQUAL(cfg.backupSpeed, outputs.rightMotorSpeed);

        // Phase 2: t=1250 (50ms <= elapsed < 150ms) -> spin right
        inputs.currentTimeMs = 1250;
        outputs = engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());
        TEST_ASSERT_EQUAL(cfg.spinSpeed, outputs.leftMotorSpeed);
        TEST_ASSERT_EQUAL(-cfg.spinSpeed, outputs.rightMotorSpeed);

        // End of evasion: t=1350 (elapsed >= 150ms) -> transitions to SEARCH
        inputs.currentTimeMs = 1350;
        outputs = engine.update(inputs);
        TEST_ASSERT_EQUAL(State::SEARCH, engine.getState());
    }

    // Test 2b: Triggered by right border -> spins left
    {
        DomainEngine engine(cfg);
        DomainInputs inputs = {0, true, false, false, false};
        engine.update(inputs); // countdown
        inputs.buttonPressed = false;
        inputs.currentTimeMs = 1000;
        engine.update(inputs); // opening move
        inputs.currentTimeMs = 1100;
        engine.update(inputs); // search

        // Right border at t=1200
        inputs.currentTimeMs = 1200;
        inputs.rightBorderDetected = true;
        DomainOutputs outputs = engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());

        // Phase 2: t=1250 -> spin left
        inputs.rightBorderDetected = false;
        inputs.currentTimeMs = 1250;
        outputs = engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());
        TEST_ASSERT_EQUAL(-cfg.spinSpeed, outputs.leftMotorSpeed);
        TEST_ASSERT_EQUAL(cfg.spinSpeed, outputs.rightMotorSpeed);
    }

    // Test 2c: Triggered by both borders -> left border behavior (spins right)
    {
        DomainEngine engine(cfg);
        DomainInputs inputs = {0, true, false, false, false};
        engine.update(inputs); // countdown
        inputs.buttonPressed = false;
        inputs.currentTimeMs = 1000;
        engine.update(inputs); // opening move
        inputs.currentTimeMs = 1100;
        engine.update(inputs); // search

        // Both borders at t=1200
        inputs.currentTimeMs = 1200;
        inputs.leftBorderDetected = true;
        inputs.rightBorderDetected = true;
        DomainOutputs outputs = engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());

        // Phase 2: t=1250 -> spins right
        inputs.leftBorderDetected = false;
        inputs.rightBorderDetected = false;
        inputs.currentTimeMs = 1250;
        outputs = engine.update(inputs);
        TEST_ASSERT_EQUAL(State::RECOVERY, engine.getState());
        TEST_ASSERT_EQUAL(cfg.spinSpeed, outputs.leftMotorSpeed);
        TEST_ASSERT_EQUAL(-cfg.spinSpeed, outputs.rightMotorSpeed);
    }
}

// 10 & 11. Ubiquitous language names & Independent execution on native builds
// Fulfilled by compiling natively and independently, using the defined domain language.

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_countdown_duration);
    RUN_TEST(test_opening_move_starts_after_countdown);
    RUN_TEST(test_opening_move_interruptible_by_detection);
    RUN_TEST(test_opening_move_transitions_to_search);
    RUN_TEST(test_search_transitions_to_charge_on_detection);
    RUN_TEST(test_charge_persistence);
    RUN_TEST(test_recovery_priority);
    RUN_TEST(test_recovery_atomicity);
    RUN_TEST(test_recovery_transitions_to_search_after_completion);
    return UNITY_END();
}
