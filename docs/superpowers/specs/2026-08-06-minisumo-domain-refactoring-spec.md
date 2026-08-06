# Specification: Minisumo Domain Refactoring & FSM Seam

## Problem Statement

The current firmware codebase uses generic and implementation-oriented naming (`STATE_EVADE_LEFT`, `STATE_ATTACK`, `STATE_INITIAL_TACTIC`, `lineReadLeft`, `distanceRead`) that does not align with official competition terminology or domain concepts (`Dohyō`, `Opponent`, `Border`, `Countdown`, `Opening Move`, `Detection`, `Loss`, `Recovery`, `Push Out`, `Search`, `Charge`). Additionally, domain state transitions (such as `Recovery` priority and atomicity) are tightly coupled with hardware driver calls (`pulseIn`, `analogWrite`), making automated host-side testing impossible.

## Solution

Refactor the firmware's architecture to align with the project's ubiquitous language defined in `CONTEXT.md`. Decouple domain logic (state machine and behaviors) from hardware IO via a single clean seam, enabling host-native unit testing in `test/` and ensuring strict adherence to domain rules (atomic `Recovery`, mandatory `Countdown`, strategic `Opening Move`, etc.).

## User Stories

1. As a minisumo competitor, I want the robot to observe a mandatory 5-second `Countdown` after activation, so that the robot complies with competition homologation regulations.
2. As a minisumo competitor, I want the robot to execute an `Opening Move` immediately after the `Countdown`, so that it rapidly sweeps for the `Opponent` at match start.
3. As a minisumo competitor, I want the `Opening Move` to be interruptible by `Detection`, so that the robot can instantly transition to `Charge` if the `Opponent` is located before the sweep finishes.
4. As a minisumo competitor, I want the robot to enter `Search` after `Opening Move` completes without `Detection`, so that it scans the `Dohyō` for the `Opponent`.
5. As a minisumo competitor, I want the robot to transition from `Search` to `Charge` upon `Detection`, so that it drives toward the `Opponent` at maximum speed to attempt a `Push Out`.
6. As a minisumo competitor, I want the robot to persist `Charge` for a brief grace period upon temporary `Loss` before reverting to `Search`, so that minor sensor noise does not disengage an active attack.
7. As a minisumo competitor, I want the robot to immediately enter `Recovery` whenever a `Border` is detected, regardless of whether it is currently in `Opening Move`, `Search`, or `Charge` (but NOT during `Countdown`, as homologation rules mandate zero movement).
8. As a minisumo competitor, I want `Recovery` to execute atomically without interruption from new `Detection` events, so that the robot safely moves away from the `Border` before making strategic decisions.
9. As a minisumo competitor, I want the robot to resume `Search` after `Recovery` completes, so that it regains orientation and locates the `Opponent` from a safe position on the `Dohyō`.
10. As a software maintainer, I want all state machine state names and function signatures in the codebase to mirror `CONTEXT.md` vocabulary, so that domain rules are clear and explicit in the code.
11. As a software maintainer, I want state machine logic to run independently of hardware drivers on native host builds, so that behavior can be validated automatically in CI/CD without physical microcontrollers.

## Implementation Decisions

- **Module Architecture**:
  - Separate firmware into a pure **Domain / FSM Engine** module and a **Hardware Driver / Platform** module.
  - The Domain Engine encapsulates states (`COUNTDOWN`, `OPENING_MOVE`, `SEARCH`, `CHARGE`, `RECOVERY`) and state transition logic.
  - The Hardware Driver module handles physical pin configuration, sensor reading polling, and raw PWM output generation.
- **Interface Contracts**:
  - Domain Engine interface consumes abstract input snapshots (sensor triggers, elapsed time ticks) and produces target behavior/motor drive intentions.
  - Hardware Driver maps physical inputs (TCRT5000 pins, HC-SR04 `pulseIn`, button) into abstract domain events, and maps motor drive intentions to TB6612FNG control pins.
- **Domain Behavior Rules**:
  - `Recovery` priority rule: `Border` detection overrides all active motion states (`Opening Move`, `Search`, `Charge`). It is explicitly ignored during `Countdown` to guarantee the 5-second mandatory stillness.
  - `Recovery` atomicity rule: `Recovery` sequence runs to completion (backup duration + spin duration) regardless of distance sensor inputs.
  - `Opening Move` rule: Fast rotation for fixed maximum duration, aborted immediately on `Detection`.
  - `Charge` persistence rule: Distance sensor loss triggers internal persistence timer before emitting `Loss` event.
- **Ubiquitous Language Renaming**:
  - Replace `STATE_EVADE_*` with `RECOVERY`
  - Replace `STATE_ATTACK` with `CHARGE`
  - Replace `STATE_INITIAL_TACTIC` with `OPENING_MOVE`
  - Replace `STATE_SAFETY_DELAY` with `COUNTDOWN`
  - All debug/serial outputs and internal logs must be standardized to English to match the ubiquitous language, while maintaining code comments in Spanish (per user preference).

## Testing Decisions

- **Good Test Criteria**: Tests must verify domain behavior and state transitions given sequences of inputs, without mocking internal state machine variables or asserting on pin bit manipulations.
- **Target Modules**: The Domain / FSM Engine module tested under `env:native` in PlatformIO.
- **Prior Art**: Unity testing framework via PlatformIO test runner in `test/`.

## Out of Scope

- Modifying physical hardware wiring or pin assignments.
- Altering the 18650 power delivery topology.
- Adding custom IR remote start modules or non-autonomous control modes.
- Modifying official competition rule PDFs in `docs/`.

## Further Notes

- `CONTEXT.md` at project root remains the authoritative glossary for all terminology used in code, comments, and specs.
