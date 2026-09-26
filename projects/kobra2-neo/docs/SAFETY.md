# Safety contract

Current machine state is intentionally headless: the stock hotend assembly and original Z probe are absent.

1. No `G28 Z`, mesh leveling or probe-based Z routines.
2. No firmware flashing until exact controller/MCU identity and recovery procedure are documented.
3. No heater or extrusion commands during motion-platform development unless explicitly required and reviewed.
4. Physical moves require operator-confirmed clearance. Start with small bounded relative moves.
5. Use `M400` when a workflow depends on confirmed planner completion.
6. Do not treat `M114` as proof of real physical position until a valid homing/reference system exists.
7. A new tool must not reduce carriage clearance, cable safety or reachable emergency access without documenting the change.
8. Clay/paste tools should keep heavy reservoirs off the moving carriage where practical.
