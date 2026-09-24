# Feature: Fix Opponent Detection and Attack

## Objective
Corregir la detección de oponentes con el sensor HC-SR04, resolver el cruce de pines en el driver TB6612FNG y restaurar la transición confiable al estado de ataque (STATE_ATTACK) en la máquina de estados.

## Problem & Context
1. El sensor HC-SR04 queda ciego (devolviendo 0 cm) en la búsqueda debido a un timeout demasiado corto (6000 us) y a la falta de recuperación cuando el pin ECHO queda en HIGH durante los ~38 ms de fin de eco al sensar al aire.
2. En `src/motors.cpp`, el commit `2bc5766` cruzó los pines de dirección y PWM: `speedA` manipula `PIN_BIN1`/`PIN_BIN2` pero modula `PIN_PWMA`, y viceversa con `speedB`, generando anomalías en giros.
3. En `src/main.cpp`, `STATE_APPROACH` reseteaba `lastKnownDist = 0` en el primer 0 del sensor, bloqueando la transición a `STATE_ATTACK` cuando el robot hace contacto físico (< 2 cm).
4. `ATTACK_DISTANCE` en staged (`25 cm`) anulaba las rampas intermedias de aproximación.

## Authorized Scope
- `src/config.h`
- `src/sensors.cpp`
- `src/motors.cpp`
- `src/main.cpp`

## Constraints & Checks
- No usar funciones bloqueantes `delay()` en combate que afecten la respuesta de línea blanca.
- Mantener la compatibilidad con el entorno PlatformIO (`pio run`).
- Conventional commits por unidad de trabajo.

## Tasks
- [x] TASK-1: Desbloqueo y calibración de timing de HC-SR04 en `src/sensors.cpp` y `src/config.h`
- [x] TASK-2: Corrección del cruce de pines de control en `src/motors.cpp`
- [x] TASK-3: Corrección de lógica de proximidad y transición a `STATE_ATTACK` en `src/main.cpp` y `src/config.h`
- [x] TASK-4: Compilación y verificación final con `pio run`
