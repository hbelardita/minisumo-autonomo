# Minisumo Autónomo

Firmware for an autonomous minisumo robot that competes in amateur minisumo competitions. The robot detects its opponent and pushes it out of the dohyō while avoiding crossing the border itself.

## Language

**Dohyō**:
The circular competition surface where the match takes place. Black, with a white painted border.
_Avoid_: ring, arena, platform

**Opponent**:
The other robot competing in the current match.
_Avoid_: rival, adversary, enemy, target

**Border**:
The white painted line at the edge of the dohyō that marks the boundary. Crossing it means leaving the dohyō.
_Avoid_: edge, line, rim, boundary

**Countdown**:
The mandatory 5-second wait after power-on before the robot may move, required by competition regulations.
_Avoid_: safety delay, start delay, ready period

**Opening Move**:
A brief, fast spin executed immediately after the Countdown to sweep for the opponent before entering Search. Distinct from Search: faster, time-limited, and interruptible by Detection.
_Avoid_: initial tactic, first sweep

**Detection**:
The event of sensing the opponent's presence within striking range via the distance sensor.
_Avoid_: sighting, contact, lock-on

**Loss**:
The event of the opponent no longer being sensed within range. The inverse of Detection.
_Avoid_: lost contact, target lost

**Recovery**:
The maneuver the robot executes when it detects the border — reversing and spinning back toward the center of the dohyō to regain a safe position. Recovery always takes priority over any other behavior, and it is atomic: once started, it runs to completion before the robot can act on new Detection events.
_Avoid_: evade, retreat, escape

**Push Out**:
The act of physically pushing the opponent beyond the border of the dohyō. This is what the robot does; the Yūkō is the point the judge awards as a result.
_Avoid_: expulsion, knockoff

**Search**:
Spinning in place to scan for the opponent when its location is unknown.
_Avoid_: scan, seek

**Charge**:
Driving forward at maximum speed toward the opponent after Detection, with the goal of achieving a Push Out.
_Avoid_: attack, rush, push
