# Kobra 2 Neo project rules

These rules apply to everything under `projects/kobra2-neo/`.

## Scope

- Keep Kobra-specific code, calibration, CAD, G-code policy and toolhead design in this project.
- Treat `host-ops` as an external generic machine/device capability layer. Do not modify `host-ops` from this project unless a genuinely reusable cross-project capability is separately justified.
- Prefer small evidence-driven changes over speculative frameworks.

## Hardware safety

- The stock print head and original Z probe are currently removed.
- Do not run `G28 Z`, mesh leveling or probe-dependent routines until a new Z-reference strategy is installed and validated.
- Do not flash firmware without an explicit requirement, exact board/MCU identification and a recovery path.
- Do not heat, extrude or energize future tool actuators unless the current task explicitly requires it.
- Before physical motion, establish operator-confirmed clearance and use bounded moves. Use `M400` when command completion must be proven.
- Never infer true physical coordinates solely from `M114` while the machine lacks a valid homing/reference contract.

## Mechanical design

- Design around a common rigid adapter/tool interface rather than a one-off pen holder.
- Keep moving mass low and close to the carriage plane.
- Preserve a repeatable datum, Z adjustment, strain relief and space for a future independent Z-reference module.
- Prefer editable source CAD. Keep generated STL/3MF exports beside or below their source design, never as the only design artifact.

## Software

- Stock Marlin is the current baseline.
- Kobra-specific G-code sequences belong here, not in generic `host-ops` capabilities.
- Safety checks and calibration state must be explicit rather than inferred from printer model defaults.
