# Safety contract

This contract describes the current pen-plotter setup. Historical headless-state restrictions in older checkpoints are superseded by the current physically verified hardware state.

1. Perform exactly one physical printer operation at a time and wait for operator confirmation before the next operation.
2. Do not enable heaters, extrusion or firmware flashing unless explicitly required and reviewed for the current task.
3. Homing is never implicit. Any `G28` requires a separate explicit decision before execution.
4. The current cylindrical sensor is verified as `z_min`, and `G28 Z` has worked with the present mechanics. Revalidate the physical setup before using Z homing after any toolhead change.
5. Automatic plotting motion must remain inside the measured pen-tip envelope `X=3..223`, `Y=36..230` mm. Normal plots use an additional 5 mm internal drawing margin.
6. Current pen-up is `Z=6.12` and current pen-down is `Z=2.97`. Revalidate both after changing the pen, holder, paper placement or mechanics.
7. Render and fully inspect generated G-code before opening a live serial execution path. Verify bounds and every non-XY profile command.
8. Reject plots containing unintended `G28`, heater commands, extrusion commands or XY moves outside the allowed envelope.
9. Never send a complete artwork to the printer without explicit operator approval after dry-run inspection.
10. Use `M400` when a workflow depends on confirmed planner completion.
11. Do not identify the printer only by CH340 VID/PID or a remembered device path. Confirm with `M115` when identity is uncertain.
12. A new tool or mechanical revision invalidates calibration values that depend on tool geometry until they are physically re-measured.
