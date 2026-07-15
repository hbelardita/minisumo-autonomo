# Design Spec: Minisumo Firmware Refactoring and Bug Fixes

## 1. Goal
Refactor the existing autonomous minisumo robot firmware to improve modularity, maintainability, and code style consistency. This includes splitting header-only inline code into standard `.h` and `.cpp` file pairs, centralizing configuration constants, and fixing critical state machine (FSM) and hardware control bugs.

## 2. Target File Structure
The project will be organized as follows:
- [src/config.h](file:///home/horacio/Documents/tmp/minisumo-autonomo/src/config.h): Centralized configuration containing all pin mappings, FSM thresholds, and calibration constants.
- [src/motors.h](file:///home/horacio/Documents/tmp/minisumo-autonomo/src/motors.h): Declarations for motor driver control functions.
- [src/motors.cpp](file:///home/horacio/Documents/tmp/minisumo-autonomo/src/motors.cpp): Implementations of motor driver functions.
- [src/sensors.h](file:///home/horacio/Documents/tmp/minisumo-autonomo/src/sensors.h): Declarations for sensor reading functions.
- [src/sensors.cpp](file:///home/horacio/Documents/tmp/minisumo-autonomo/src/sensors.cpp): Implementations of sensor reading and filtering logic.
- [src/main.cpp](file:///home/horacio/Documents/tmp/minisumo-autonomo/src/main.cpp): Arduino setup, main loop, and FSM transition logic.

---

## 3. Bug Fixes & Improvements

### A. FSM Attack State Reset (`timeLost` Bug)
- **Problem**: The static local variable `timeLost` in `STATE_ATTACK` retains its value across state transitions. If the state is exited while the timer is counting, the next entry into `STATE_ATTACK` will immediately time out on the first sensor miss.
- **Solution**: Move `timeLost` to a file-scope static global in `src/main.cpp` and reset it to `0` inside `transitionTo()` when entering `STATE_ATTACK`.

### B. Motor Driver Active during Standby
- **Problem**: In `STATE_STANDBY`, `motorsBrake()` is called which sets the standby pin `STBY` to `HIGH` and brakes the motors. This consumes standby current and locks the wheels, making it hard to position the robot.
- **Solution**: Add `motorsStandby()` which sets `STBY` to `LOW` (disabling the driver and allowing the wheels to coast). Call this function in `STATE_STANDBY`.

### C. Pin Naming Consistency
- **Problem**: Motor pins (`STBY`, `AIN1`, etc.) lack the `PIN_` prefix, unlike sensor/UI pins (`PIN_TRIG`, `PIN_ECHO`).
- **Solution**: Add the `PIN_` prefix to all motor driver pins (e.g. `PIN_STBY`, `PIN_AIN1`, `PIN_AIN2`, `PIN_PWMA`, `PIN_BIN1`, `PIN_BIN2`, `PIN_PWMB`).

---

## 4. Module Specifications

### `src/config.h`
```cpp
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Motor Driver (TB6612FNG) Pins
extern const uint8_t PIN_STBY;
extern const uint8_t PIN_AIN1;
extern const uint8_t PIN_AIN2;
extern const uint8_t PIN_PWMA;
extern const uint8_t PIN_BIN1;
extern const uint8_t PIN_BIN2;
extern const uint8_t PIN_PWMB;

// HC-SR04 Pins
extern const uint8_t PIN_TRIG;
extern const uint8_t PIN_ECHO;

// TCRT5000 Pins
extern const uint8_t PIN_TCRT_LEFT;
extern const uint8_t PIN_TCRT_RIGHT;

// UI Pins
extern const uint8_t PIN_BUTTON;
extern const uint8_t PIN_LED;

// Calibration & Thresholds
const unsigned int ATTACK_DISTANCE = 50;   // Distance threshold (cm) to charge
const unsigned long TACTIC_MS = 400;       // Duration (ms) of initial spin tactic
const unsigned long BACKUP_MS = 250;       // Backup duration (ms) on border evasion
const unsigned long EVADE_MS = 450;        // Total evasion duration (ms) (backup + spin)
const unsigned long PERSIST_MS = 200;      // Attack persistence (ms) after losing target
const unsigned long SAFETY_MS = 5000;      // Mandatory safety start delay (ms)
const unsigned long BLINK_MS = 100;        // Blink interval (ms) during countdown
const unsigned long ULTRASONIC_TIMEOUT = 3500; // pulseIn timeout (us)

// Speeds
const int EVADE_BACKUP_SPEED = -180;
const int EVADE_SPIN_SPEED = 180;
const int INITIAL_TACTIC_SPEED = 160;
const int SEARCH_SPEED = 120;
const int ATTACK_SPEED = 255;

#endif
```

### `src/motors.h`
```cpp
#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>

void motorsInit();
void motorsSetSpeed(int leftSpeed, int rightSpeed);
void motorsBrake();
void motorsStandby();

#endif
```

### `src/sensors.h`
```cpp
#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

void sensorsInit();
unsigned int distanceRead();
bool lineReadLeft();
bool lineReadRight();
bool buttonPressed();

#endif
```
