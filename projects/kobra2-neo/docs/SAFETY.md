# Safety contract

This contract describes the current dedicated pen-plotter setup. The original printhead / hotend assembly is intentionally removed: there is no hotend heater cartridge, no hotend thermistor and no printhead fan hardware. The active tool is a pen/marker, and the cylindrical magnetic/proximity sensor is the verified Z homing reference (`z_min`).

1. After the complete plot, calibration, bounds and start sequence have been reviewed, one explicit operator approval may authorize the full start-and-draw transaction: `G28 X Y`, `G28 Z`, pen-up, travel to the first plot point, complete artwork streaming, final `M400`, and end pen-up. Intermediate confirmations are not required unless the operator asks to pause or an error/unsafe condition occurs.
2. Do not enable heater commands, extrusion or firmware flashing unless explicitly required and reviewed for a future hardware revision. The current plotter has no hotend heater hardware.
3. Homing is explicit at the job level, not implicit in prepared artwork. `G28` must remain absent from prepare-generated G-code and may only be added by the live execution preamble after the full transaction has been approved.
4. The current cylindrical magnetic/proximity sensor is verified as `z_min`, and `G28 Z` has worked with the present pen mechanics. Revalidate the physical setup before using Z homing after any pen-holder/tool geometry change.
5. Automatic plotting motion must remain inside the measured pen-tip envelope `X=3..223`, `Y=36..230` mm. Normal plots use an additional 5 mm internal drawing margin.
6. Current pen-up is `Z=6.12` and current pen-down is `Z=2.97`. Revalidate both after changing the pen, holder, paper placement or mechanics.
7. Render and fully inspect generated G-code before opening a live serial execution path. Verify bounds and every non-XY profile command.
8. Reject prepared plots containing unintended `G28`, heater commands, extrusion commands or XY moves outside the allowed envelope.
9. Never send a complete artwork to the printer without explicit operator approval after dry-run inspection.
10. Use `M400` when a workflow depends on confirmed planner completion.
11. Do not identify the printer only by CH340 VID/PID or a remembered device path. The live executor must confirm with `M115` before motion.
12. A new tool or mechanical revision invalidates calibration values that depend on tool geometry until they are physically re-measured.
13. The absent hotend thermistor is intentional. An `M105` hotend value of `T:0.00` is expected and **must not block pen plotting**. Do not require a plausible room-temperature hotend reading in this hardware profile.
14. Do not use nozzle-temperature controls on the printer screen or send heater G-code during pen plotting. Because the thermistor is intentionally absent, attempting to heat the nonexistent hotend may make stock Marlin enter a thermal fault/kill state.
15. A firmware-reported `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or equivalent halt **during an active transaction** remains a terminal stop condition. Do not ignore a firmware kill simply because the sensor is intentionally absent.
16. After a firmware kill/halt, do not claim that a recovery pen-up succeeded and do not repeatedly send motion commands. The final pen state is unknown until the machine is physically inspected/reset and safe motion is re-established.
17. Before `G28 X Y`, verify that the full Y-bed path is physically clear. In particular, keep the printer power cable out of the rear travel path; on 2026-09-29 it mechanically blocked the bed before `y_min` until the cable was moved.
