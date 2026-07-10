# Minisumo Autonomous Controller Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Create a responsive, non-blocking autonomous minisumo controller firmware in PlatformIO for Arduino Nano, integrating TB6612FNG motor control, HC-SR04 ultrasonic distance sensing, dual TCRT5000 line sensors, a start button, and a status LED.

**Architecture:** A cooperative multi-tasking Finite State Machine (FSM) implemented in C++ running inside a fast, non-blocking Arduino execution loop. Time-based operations use `millis()` and `micros()`, ultrasonic scanning uses a low-timeout `pulseIn` constraint, and line sensors use analog thresholding.

**Tech Stack:** Arduino Framework (C++), PlatformIO CLI.

## Global Constraints

*   **Processor:** ATmega328P (Arduino Nano).
*   **Timeouts:** Ultrasonic distance reading must not block the loop for more than 6ms (6000μs timeout).
*   **Line Sensors:** Low values (e.g. < 400) represent the white line; high values represent the black dohyo.
*   **Safety Delay:** Must wait exactly 5000ms after button trigger before enabling motors. LED must blink during this period.
*   **Initial Tactic:** Spin search for up to 400ms or until opponent is detected before entering standard search.

---

## Tasks

### Task 1: Project Configuration & Verification Setup

**Files:**
*   Modify: `platformio.ini`
*   Create: `src/main.cpp`

**Interfaces:**
*   Produces: Defines pin mappings and initial compiler validation verification.

- [ ] **Step 1: Configure PlatformIO Environment**
  Update `platformio.ini` to define the environment for Arduino Nano with the standard ATmega328P microcontroller.

  Replace `platformio.ini` content with:
  ```ini
  [env:nanoatmega328]
  platform = atmelavr
  board = nanoatmega328
  framework = arduino
  ```

- [ ] **Step 2: Create Main File with Pinout Constants**
  Create `src/main.cpp` with the exact pin allocations provided by the hardware layout.

  Create `src/main.cpp` with:
  ```cpp
  #include <Arduino.h>

  // TB6612FNG Pins
  const int STBY = 8;
  const int AIN1 = 7;
  const int AIN2 = 6;
  const int PWMA = 5;
  const int BIN1 = 4;
  const int BIN2 = 2;
  const int PWMB = 9;

  // HC-SR04 Pins
  const int PIN_TRIG = 11;
  const int PIN_ECHO = 12;

  // TCRT5000 Pins
  const int PIN_TCRT_LEFT = A2;
  const int PIN_TCRT_RIGHT = A3;

  // UI Pins
  const int PIN_BUTTON = 10;
  const int PIN_LED = 13;

  void setup() {
      pinMode(PIN_LED, OUTPUT);
      pinMode(PIN_BUTTON, INPUT_PULLUP);
      digitalWrite(PIN_LED, LOW);
  }

  void loop() {
      // Basic sanity blink
      digitalWrite(PIN_LED, HIGH);
      delay(100);
      digitalWrite(PIN_LED, LOW);
      delay(100);
  }
  ```

- [ ] **Step 3: Verify Compilation**
  Run: `pio run`
  Expected: Success, binary compiled successfully.

---

### Task 2: Motor Control Driver Implementation

**Files:**
*   Create: `src/motors.h`
*   Modify: `src/main.cpp`

**Interfaces:**
*   Produces: `void motorsInit()`, `void motorsSetSpeed(int leftSpeed, int rightSpeed)`, `void motorsBrake()`

- [ ] **Step 1: Implement Motor Driver Header**
  Create `src/motors.h` to declare pin modes and implement speed control functions mapping values of `-255` to `255` to forward/backward direction and PWM signals.

  Create `src/motors.h`:
  ```cpp
  #ifndef MOTORS_H
  #define MOTORS_H

  #include <Arduino.h>

  extern const int STBY;
  extern const int AIN1;
  extern const int AIN2;
  extern const int PWMA;
  extern const int BIN1;
  extern const int BIN2;
  extern const int PWMB;

  inline void motorsInit() {
      pinMode(STBY, OUTPUT);
      pinMode(AIN1, OUTPUT);
      pinMode(AIN2, OUTPUT);
      pinMode(PWMA, OUTPUT);
      pinMode(BIN1, OUTPUT);
      pinMode(BIN2, OUTPUT);
      pinMode(PWMB, OUTPUT);
      
      digitalWrite(STBY, LOW); // Disable driver by default
  }

  inline void motorsSetSpeed(int leftSpeed, int rightSpeed) {
      digitalWrite(STBY, HIGH); // Enable driver

      // Left Motor (Motor A)
      if (leftSpeed > 0) {
          digitalWrite(AIN1, HIGH);
          digitalWrite(AIN2, LOW);
          analogWrite(PWMA, leftSpeed);
      } else if (leftSpeed < 0) {
          digitalWrite(AIN1, LOW);
          digitalWrite(AIN2, HIGH);
          analogWrite(PWMA, -leftSpeed);
      } else {
          digitalWrite(AIN1, LOW);
          digitalWrite(AIN2, LOW);
          analogWrite(PWMA, 0);
      }

      // Right Motor (Motor B)
      if (rightSpeed > 0) {
          digitalWrite(BIN1, HIGH);
          digitalWrite(BIN2, LOW);
          analogWrite(PWMB, rightSpeed);
      } else if (rightSpeed < 0) {
          digitalWrite(BIN1, LOW);
          digitalWrite(BIN2, HIGH);
          analogWrite(PWMB, -rightSpeed);
      } else {
          digitalWrite(BIN1, LOW);
          digitalWrite(BIN2, LOW);
          analogWrite(PWMB, 0);
      }
  }

  inline void motorsBrake() {
      digitalWrite(STBY, HIGH);
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, HIGH);
      analogWrite(PWMA, 0);
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, HIGH);
      analogWrite(PWMB, 0);
  }

  #endif
  ```

- [ ] **Step 2: Integrate Motors Init in Main**
  Modify `src/main.cpp` to include the motors header and initialize them in the setup routine.

  Replace `src/main.cpp` content with:
  ```cpp
  #include <Arduino.h>
  #include "motors.h"

  // TB6612FNG Pins
  const int STBY = 8;
  const int AIN1 = 7;
  const int AIN2 = 6;
  const int PWMA = 5;
  const int BIN1 = 4;
  const int BIN2 = 2;
  const int PWMB = 9;

  // HC-SR04 Pins
  const int PIN_TRIG = 11;
  const int PIN_ECHO = 12;

  // TCRT5000 Pins
  const int PIN_TCRT_LEFT = A2;
  const int PIN_TCRT_RIGHT = A3;

  // UI Pins
  const int PIN_BUTTON = 10;
  const int PIN_LED = 13;

  void setup() {
      pinMode(PIN_LED, OUTPUT);
      pinMode(PIN_BUTTON, INPUT_PULLUP);
      digitalWrite(PIN_LED, LOW);
      motorsInit();
  }

  void loop() {
      // Test motor spin forward briefly
      motorsSetSpeed(100, 100);
      delay(500);
      motorsBrake();
      delay(1000);
  }
  ```

- [ ] **Step 3: Compile Verification**
  Run: `pio run`
  Expected: Success.

---

### Task 3: Sensor Interfacing & Acquisition Implementation

**Files:**
*   Create: `src/sensors.h`
*   Modify: `src/main.cpp`

**Interfaces:**
*   Produces: `void sensorsInit()`, `unsigned int distanceRead()`, `int lineReadLeft()`, `int lineReadRight()`, `bool buttonPressed()`

- [ ] **Step 1: Create Sensor Driver Module**
  Create `src/sensors.h` implementing line readings, non-blocking button state checks, and the low-latency ultrasonic trigger routine using a 6000μs timeout constraint on `pulseIn`.

  Create `src/sensors.h`:
  ```cpp
  #ifndef SENSORS_H
  #define SENSORS_H

  #include <Arduino.h>

  extern const int PIN_TRIG;
  extern const int PIN_ECHO;
  extern const int PIN_TCRT_LEFT;
  extern const int PIN_TCRT_RIGHT;
  extern const int PIN_BUTTON;

  inline void sensorsInit() {
      pinMode(PIN_TRIG, OUTPUT);
      pinMode(PIN_ECHO, INPUT);
      pinMode(PIN_TCRT_LEFT, INPUT);
      pinMode(PIN_TCRT_RIGHT, INPUT);
      
      digitalWrite(PIN_TRIG, LOW);
  }

  // Returns distance in cm, or 0 if timeout (no object within ~1m)
  inline unsigned int distanceRead() {
      // Send 10 microsecond trigger pulse
      digitalWrite(PIN_TRIG, LOW);
      delayMicroseconds(2);
      digitalWrite(PIN_TRIG, HIGH);
      delayMicroseconds(10);
      digitalWrite(PIN_TRIG, LOW);

      // Read pulse duration with a 6000 microsecond limit (~100cm max range)
      unsigned long duration = pulseIn(PIN_ECHO, HIGH, 6000);
      if (duration == 0) {
          return 0; // No obstacle in range
      }
      return duration * 0.0343 / 2.0;
  }

  inline int lineReadLeft() {
      return analogRead(PIN_TCRT_LEFT);
  }

  inline int lineReadRight() {
      return analogRead(PIN_TCRT_RIGHT);
  }

  inline bool buttonPressed() {
      return digitalRead(PIN_BUTTON) == LOW;
  }

  #endif
  ```

- [ ] **Step 2: Clean Setup Function**
  Update `src/main.cpp` to initialize all sensors.

  Replace `src/main.cpp` content with:
  ```cpp
  #include <Arduino.h>
  #include "motors.h"
  #include "sensors.h"

  // Pin Declarations
  const int STBY = 8;
  const int AIN1 = 7;
  const int AIN2 = 6;
  const int PWMA = 5;
  const int BIN1 = 4;
  const int BIN2 = 2;
  const int PWMB = 9;

  const int PIN_TRIG = 11;
  const int PIN_ECHO = 12;

  const int PIN_TCRT_LEFT = A2;
  const int PIN_TCRT_RIGHT = A3;

  const int PIN_BUTTON = 10;
  const int PIN_LED = 13;

  void setup() {
      pinMode(PIN_LED, OUTPUT);
      digitalWrite(PIN_LED, LOW);
      
      motorsInit();
      sensorsInit();
  }

  void loop() {
      // Test read
      unsigned int dist = distanceRead();
      if (dist > 0 && dist < 50) {
          digitalWrite(PIN_LED, HIGH);
      } else {
          digitalWrite(PIN_LED, LOW);
      }
      delay(50);
  }
  ```

- [ ] **Step 3: Compile Verification**
  Run: `pio run`
  Expected: Success.

---

### Task 4: Finite State Machine Control Logic

**Files:**
*   Modify: `src/main.cpp`

**Interfaces:**
*   Consumes: `src/motors.h` and `src/sensors.h`
*   Produces: Execution loop and state transitions.

- [ ] **Step 1: Write Full Control Loop & State Logic**
  Update `src/main.cpp` to implement all FSM states with the blinking safety LED, initial tactic, search sweep, line detection interrupts, and chase behavior.

  Replace `src/main.cpp` content with:
  ```cpp
  #include <Arduino.h>
  #include "motors.h"
  #include "sensors.h"

  // Pins
  const int STBY = 8;
  const int AIN1 = 7;
  const int AIN2 = 6;
  const int PWMA = 5;
  const int BIN1 = 4;
  const int BIN2 = 2;
  const int PWMB = 9;

  const int PIN_TRIG = 11;
  const int PIN_ECHO = 12;

  const int PIN_TCRT_LEFT = A2;
  const int PIN_TCRT_RIGHT = A3;

  const int PIN_BUTTON = 10;
  const int PIN_LED = 13;

  // Calibration Thresholds
  const int LINE_THRESHOLD = 400; // Low reading represents white line
  const unsigned int ATTACK_DISTANCE = 50; // Threshold in cm to trigger attack
  const unsigned long INITIAL_TACTIC_DURATION = 400; // Spin duration in ms

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
  }

  void setup() {
      pinMode(PIN_LED, OUTPUT);
      digitalWrite(PIN_LED, LOW);
      
      motorsInit();
      sensorsInit();
      
      transitionTo(STATE_STANDBY);
  }

  void loop() {
      // Priority 1: Check Line Sensors (only if we are active in combat)
      if (currentState != STATE_STANDBY && currentState != STATE_SAFETY_DELAY) {
          if (lineReadLeft() < LINE_THRESHOLD) {
              transitionTo(STATE_EVADE_LEFT);
          } else if (lineReadRight() < LINE_THRESHOLD) {
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
              if (millis() - lastLedBlinkTime >= 100) {
                  lastLedBlinkTime = millis();
                  ledState = !ledState;
                  digitalWrite(PIN_LED, ledState ? HIGH : LOW);
              }

              // After 5 seconds, start the match
              if (millis() - stateStartTime >= 5000) {
                  digitalWrite(PIN_LED, HIGH); // Steady ON when active
                  transitionTo(STATE_INITIAL_TACTIC);
              }
              break;

          case STATE_INITIAL_TACTIC:
              // Spin clockwise looking for opponent
              motorsSetSpeed(160, -160);

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
              motorsSetSpeed(120, -120);

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
              motorsSetSpeed(255, 255);

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
              if (millis() - stateStartTime < 250) {
                  // Backup
                  motorsSetSpeed(-180, -180);
              } else if (millis() - stateStartTime < 450) {
                  // Spin Right
                  motorsSetSpeed(180, -180);
              } else {
                  transitionTo(STATE_SEARCH);
              }
              break;

          case STATE_EVADE_RIGHT:
              // White line on the right. Backup and turn left.
              if (millis() - stateStartTime < 250) {
                  // Backup
                  motorsSetSpeed(-180, -180);
              } else if (millis() - stateStartTime < 450) {
                  // Spin Left
                  motorsSetSpeed(-180, 180);
              } else {
                  transitionTo(STATE_SEARCH);
              }
              break;
      }
  }
  ```

- [ ] **Step 2: Final Verification Compilation**
  Run: `pio run`
  Expected: Success, 100% complete and valid firmware.
