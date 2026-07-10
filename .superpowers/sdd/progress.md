- [x] Task 1: Project Configuration & Verification Setup (commits 9cb6b1c..27879da, review clean)
- [x] Task 2: Motor Control Driver Implementation (commits 27879da..9c34ee4, review clean after fix)
- [x] Task 3: Sensor Interfacing & Acquisition Implementation (commits 9c34ee4..2026565, review clean)
- [ ] Task 4: Finite State Machine Control Logic

### Minor Findings / Triaged Issues
*   Task 1: Consider changing pin type declarations from `const int` to `constexpr uint8_t` to optimize SRAM on ATmega328P.

