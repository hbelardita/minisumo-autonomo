- [x] Task 1: Project Configuration & Verification Setup (commits 9cb6b1c..27879da, review clean)
- [x] Task 2: Motor Control Driver Implementation (commits 27879da..9c34ee4, review clean after fix)
- [x] Task 3: Sensor Interfacing & Acquisition Implementation (commits 9c34ee4..e153192, review clean)
- [x] Task 4: Finite State Machine Control Logic (commits e153192..bcf7e24, review clean after fix)

### Minor Findings / Triaged Issues
*   Task 1: Consider changing pin type declarations from `const int` to `constexpr uint8_t` to optimize SRAM on ATmega328P. (Done in Task 2)
*   Task 3: Replace floating point math `duration * 0.0343 / 2.0` with `duration / 58` in distanceRead() to save ~1.5KB of Flash and improve execution speed on AVR. (Done in Task 4)
*   Task 4: Consider adding hysteresis for the opponent detection distance threshold (e.g. detect at < 50cm, release at > 55cm) to prevent motor jitter on distance boundaries.
