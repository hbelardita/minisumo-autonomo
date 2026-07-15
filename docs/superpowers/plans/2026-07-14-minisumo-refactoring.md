# Minisumo Refactoring and Bug Fixes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Refactor the minisumo robot firmware into a modular structure (separating interfaces and implementations) and fix FSM state persistence and motor standby bugs.

**Architecture:** Split inline implementations in header files into dedicated source files (`.cpp`). Centralize pin configuration and threshold constants into a unified configuration header. Implement state variables globally and fix state logic issues.

**Tech Stack:** C++ / Arduino Framework / PlatformIO / AVR Toolchain

## Global Constraints

- All generated code comments in header and source files must be in Spanish.
- PlatformIO project targeting Arduino Nano (`nanoatmega328` board, `atmelavr` platform, `arduino` framework).
- Pin mapping must use the `PIN_` prefix consistently for both motors and sensors.
- Code implementations must be separated into `.cpp` files, and headers (`.h` files) must contain only declarations.
- Variable `timeLost` must be reset to `0` on transition to `STATE_ATTACK`.
- Standby mode must use `motorsStandby()` (setting `STBY` to `LOW`) during `STATE_STANDBY`.

---

### Task 1: Centralize Configuration

**Files:**
- Create: `src/config.h`
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: None
- Produces: Pin mapping declarations and FSM thresholds/calibration constants.

- [ ] **Step 1: Create `src/config.h`**
  Write the centralized configuration header with `extern` declarations for hardware pins and constants for timeouts/speeds. All comments must be in Spanish.
  
  ```cpp
  #ifndef CONFIG_H
  #define CONFIG_H

  #include <Arduino.h>

  // Pines del Driver de Motores (TB6612FNG)
  extern const uint8_t PIN_STBY;
  extern const uint8_t PIN_AIN1;
  extern const uint8_t PIN_AIN2;
  extern const uint8_t PIN_PWMA;
  extern const uint8_t PIN_BIN1;
  extern const uint8_t PIN_BIN2;
  extern const uint8_t PIN_PWMB;

  // Pines del Sensor Ultrasónico (HC-SR04)
  extern const uint8_t PIN_TRIG;
  extern const uint8_t PIN_ECHO;

  // Pines de Sensores de Línea (TCRT5000)
  extern const uint8_t PIN_TCRT_LEFT;
  extern const uint8_t PIN_TCRT_RIGHT;

  // Pines de la Interfaz de Usuario (UI)
  extern const uint8_t PIN_BUTTON;
  extern const uint8_t PIN_LED;

  // Umbrales y Tiempos de Calibración
  const unsigned int ATTACK_DISTANCE = 50;   // Distancia en cm para atacar
  const unsigned long TACTIC_MS = 400;       // Tiempo de giro inicial de la táctica
  const unsigned long BACKUP_MS = 250;       // Tiempo de retroceso en evasión
  const unsigned long EVADE_MS = 450;        // Tiempo total de evasión (retroceso + giro)
  const unsigned long PERSIST_MS = 200;      // Tiempo de persistencia de ataque sin ver al rival
  const unsigned long SAFETY_MS = 5000;      // Tiempo de espera de seguridad inicial (5 segundos)
  const unsigned long BLINK_MS = 100;        // Frecuencia de parpadeo del LED (5Hz)
  const unsigned long ULTRASONIC_TIMEOUT = 3500; // Timeout de pulseIn (microsegundos) para ~60cm

  // Velocidades de Motores
  const int EVADE_BACKUP_SPEED = -180;
  const int EVADE_SPIN_SPEED = 180;
  const int INITIAL_TACTIC_SPEED = 160;
  const int SEARCH_SPEED = 120;
  const int ATTACK_SPEED = 255;

  #endif
  ```

- [ ] **Step 2: Update definitions in `src/main.cpp`**
  Modify [src/main.cpp](file:///home/horacio/Documents/tmp/minisumo-autonomo/src/main.cpp#L9-L27) to define the actual values for the pin mappings declared in `src/config.h`, and include `"config.h"`. Ensure the `extern` keyword is removed from the definitions.
  
  ```cpp
  #include "config.h"

  // Definiciones de pines de hardware
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
  ```

- [ ] **Step 3: Verify compilation**
  Run compilation to make sure `src/config.h` is correctly referenced and compiles without issues.
  
  Run: `pio run`
  Expected: SUCCESS

- [ ] **Step 4: Commit changes**
  Add and commit the new configuration header and modified main.cpp.
  
  Run:
  ```bash
  git add src/config.h src/main.cpp
  git commit -m "feat: centralize hardware configuration and pin mappings"
  ```

---

### Task 2: Refactor Motor Driver Module

**Files:**
- Create: `src/motors.cpp`
- Modify: `src/motors.h`

**Interfaces:**
- Consumes: `src/config.h`
- Produces: Functions `motorsInit()`, `motorsSetSpeed(int, int)`, `motorsBrake()`, `motorsStandby()`.

- [ ] **Step 1: Update `src/motors.h` declarations**
  Replace the entire contents of [src/motors.h](file:///home/horacio/Documents/tmp/minisumo-autonomo/src/motors.h) to contain only the declarations of the motor functions. Remove inline implementations and extern pin declarations. Use Spanish comments.
  
  ```cpp
  #ifndef MOTORS_H
  #define MOTORS_H

  #include <Arduino.h>

  // Inicializa los pines del driver de motores
  void motorsInit();

  // Establece la velocidad de los motores izquierdo y derecho (rango -255 a 255)
  void motorsSetSpeed(int leftSpeed, int rightSpeed);

  // Aplica freno activo cortocircuitando los motores
  void motorsBrake();

  // Desactiva el driver (standby) para permitir el movimiento libre (coasting) y ahorrar energía
  void motorsStandby();

  #endif
  ```

- [ ] **Step 2: Create `src/motors.cpp`**
  Create the source file `src/motors.cpp` with the implementations of the motor control functions. Reference the renamed pin variables (with `PIN_` prefix) and include Spanish comments.
  
  ```cpp
  #include "motors.h"
  #include "config.h"

  // Inicialización de los pines de motor como salidas
  void motorsInit() {
      pinMode(PIN_STBY, OUTPUT);
      pinMode(PIN_AIN1, OUTPUT);
      pinMode(PIN_AIN2, OUTPUT);
      pinMode(PIN_PWMA, OUTPUT);
      pinMode(PIN_BIN1, OUTPUT);
      pinMode(PIN_BIN2, OUTPUT);
      pinMode(PIN_PWMB, OUTPUT);
      
      // Desactiva el driver por defecto
      digitalWrite(PIN_STBY, LOW);
  }

  // Función auxiliar interna para configurar cada motor
  static void setMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int speed) {
      speed = constrain(speed, -255, 255);
      if (speed > 0) {
          digitalWrite(in1, HIGH);
          digitalWrite(in2, LOW);
          analogWrite(pwm, speed);
      } else if (speed < 0) {
          digitalWrite(in1, LOW);
          digitalWrite(in2, HIGH);
          analogWrite(pwm, -speed);
      } else {
          digitalWrite(in1, LOW);
          digitalWrite(in2, LOW);
          analogWrite(pwm, 0);
      }
  }

  // Establece la velocidad de ambos motores
  void motorsSetSpeed(int leftSpeed, int rightSpeed) {
      digitalWrite(PIN_STBY, HIGH); // Activa el driver
      setMotor(PIN_AIN1, PIN_AIN2, PIN_PWMA, leftSpeed);
      setMotor(PIN_BIN1, PIN_BIN2, PIN_PWMB, rightSpeed);
  }

  // Freno activo (corto circuito a VCC/GND para frenar rápido)
  void motorsBrake() {
      digitalWrite(PIN_STBY, HIGH);
      digitalWrite(PIN_AIN1, HIGH);
      digitalWrite(PIN_AIN2, HIGH);
      analogWrite(PIN_PWMA, 0);
      digitalWrite(PIN_BIN1, HIGH);
      digitalWrite(PIN_BIN2, HIGH);
      analogWrite(PIN_PWMB, 0);
  }

  // Desactiva el pin STBY para dejar los motores sueltos
  void motorsStandby() {
      digitalWrite(PIN_STBY, LOW);
  }
  ```

- [ ] **Step 3: Verify compilation**
  Run compilation to verify that motors module complies with the config mappings.
  
  Run: `pio run`
  Expected: SUCCESS

- [ ] **Step 4: Commit changes**
  Add and commit the modified motors header and new source file.
  
  Run:
  ```bash
  git add src/motors.h src/motors.cpp
  git commit -m "refactor: split motors interface and implementation"
  ```

---

### Task 3: Refactor Sensors Module

**Files:**
- Create: `src/sensors.cpp`
- Modify: `src/sensors.h`

**Interfaces:**
- Consumes: `src/config.h`
- Produces: Functions `sensorsInit()`, `distanceRead()`, `lineReadLeft()`, `lineReadRight()`, `buttonPressed()`.

- [ ] **Step 1: Update `src/sensors.h` declarations**
  Replace the entire contents of [src/sensors.h](file:///home/horacio/Documents/tmp/minisumo-autonomo/src/sensors.h) to contain only the declarations of sensor reading functions. Remove inline implementations and extern pin declarations. Use Spanish comments.
  
  ```cpp
  #ifndef SENSORS_H
  #define SENSORS_H

  #include <Arduino.h>

  // Inicializa los pines de los sensores (Trigger, Echo, TCRT y Botón)
  void sensorsInit();

  // Lee la distancia con el ultrasónico y devuelve el valor en cm (0 si hay timeout)
  unsigned int distanceRead();

  // Verifica el estado del sensor de línea izquierdo con filtro antirrebote
  bool lineReadLeft();

  // Verifica el estado del sensor de línea derecho con filtro antirrebote
  bool lineReadRight();

  // Devuelve verdadero si el botón físico está presionado (filtro activo-bajo)
  bool buttonPressed();

  #endif
  ```

- [ ] **Step 2: Create `src/sensors.cpp`**
  Create the source file `src/sensors.cpp` with the implementations of the sensor control and reading functions. Reference the renamed pin variables (with `PIN_` prefix) and write all comments in Spanish.
  
  ```cpp
  #include "sensors.h"
  #include "config.h"

  // Configuración de modos de pin para los sensores
  void sensorsInit() {
      pinMode(PIN_TRIG, OUTPUT);
      pinMode(PIN_ECHO, INPUT);
      pinMode(PIN_TCRT_LEFT, INPUT);
      pinMode(PIN_TCRT_RIGHT, INPUT);
      pinMode(PIN_BUTTON, INPUT_PULLUP);
      
      digitalWrite(PIN_TRIG, LOW);
  }

  // Envía un pulso ultrasónico y mide el tiempo de respuesta
  unsigned int distanceRead() {
      // Genera pulso de disparo de 10us
      digitalWrite(PIN_TRIG, LOW);
      delayMicroseconds(2);
      digitalWrite(PIN_TRIG, HIGH);
      delayMicroseconds(10);
      digitalWrite(PIN_TRIG, LOW);

      // Lee la duración del pulso en Echo
      unsigned long duration = pulseIn(PIN_ECHO, HIGH, ULTRASONIC_TIMEOUT);
      if (duration == 0) {
          return 0; // Sin obstáculo detectado en el rango configurado
      }
      return duration / 58;
  }

  // Función auxiliar interna para aplicar filtro antirrebote (debounce) a los TCRT5000
  static bool readLineSensor(uint8_t pin, int &count) {
      count = (digitalRead(pin) == LOW) ? count + 1 : 0;
      if (count > 4) count = 4;
      return count >= 4;
  }

  // Lectura con debounce del sensor izquierdo
  bool lineReadLeft() {
      static int count = 0;
      return readLineSensor(PIN_TCRT_LEFT, count);
  }

  // Lectura con debounce del sensor derecho
  bool lineReadRight() {
      static int count = 0;
      return readLineSensor(PIN_TCRT_RIGHT, count);
  }

  // Comprueba estado del botón (activo-bajo)
  bool buttonPressed() {
      return digitalRead(PIN_BUTTON) == LOW;
  }
  ```

- [ ] **Step 3: Verify compilation**
  Run compilation to verify that the sensors module compiles without errors.
  
  Run: `pio run`
  Expected: SUCCESS

- [ ] **Step 4: Commit changes**
  Add and commit the modified sensors header and new source file.
  
  Run:
  ```bash
  git add src/sensors.h src/sensors.cpp
  git commit -m "refactor: split sensors interface and implementation"
  ```

---

### Task 4: Refactor Main Logic & Fix State Machine Bug

**Files:**
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: `src/config.h`, `src/motors.h`, `src/sensors.h`
- Produces: Arduino entrypoints `setup()` and `loop()`.

- [ ] **Step 1: Replace `src/main.cpp` with refactored code**
  Rewrite [src/main.cpp](file:///home/horacio/Documents/tmp/minisumo-autonomo/src/main.cpp) to consume the modular header files, move `timeLost` to a static file-scope variable, reset it inside `transitionTo()` on `STATE_ATTACK` entry, call `motorsStandby()` in `STATE_STANDBY`, and write all comments in Spanish.
  
  ```cpp
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
  ```

- [ ] **Step 2: Verify compilation**
  Run final compilation checks on the completed refactored firmware.
  
  Run: `pio run`
  Expected: SUCCESS

- [ ] **Step 3: Commit changes**
  Add and commit the refactored main logic file.
  
  Run:
  ```bash
  git add src/main.cpp
  git commit -m "refactor: integrate config and modules with FSM bug fixes"
  ```
