# Kobra 2 Neo project rules

These rules apply to everything under `projects/kobra2-neo/`.

## Scope

- Keep Kobra-specific hardware state, calibration, CAD, G-code policy and toolhead design in this project.
- Treat `host-ops` as an external generic machine/device capability layer. Do not put Kobra-specific plotting policy into `host-ops` core.
- The current `host-ops/prototype/penplotter` is an external prototype dependency, not the canonical home of Kobra hardware state.
- Prefer small evidence-driven changes over speculative frameworks.

## Live hardware safety

- Do not enable heaters, extrusion or firmware flashing unless the current task explicitly requires and reviews it.
- A fully reviewed live plot may be approved as one complete physical transaction: home XY, home Z, raise the pen, travel to the first plot point, stream the complete approved job, wait for final `M400`, and finish pen-up. Do not require intermediate confirmations inside that approved flow unless the operator asks to pause or an error/unsafe condition occurs.
- The whole start-and-draw transaction still requires explicit operator approval before it begins. Keep `G28` out of normal prepare-generated G-code; the live executor owns the explicit homing preamble and must never add it without job-level approval.
- The current cylindrical sensor is verified as `z_min`, and `G28 Z` has succeeded with the present setup, but revalidate the mechanical configuration before any future Z homing after a hardware/tool change.
- Current automatic plotting bounds are the real pen-tip envelope `X=3..223`, `Y=36..230` mm. Normal plots use an additional 5 mm internal margin.
- Current pen-up is `Z=6.12`; current pen-down is `Z=2.97`. Any pen, holder, paper or mechanical change invalidates these values until revalidated.
- Render and inspect complete G-code before opening a live execution path. Never send a full image/plot without explicit operator approval.
- Use `M400` when command completion must be proven.
- Do not identify the printer solely by CH340 VID/PID or a remembered serial path; verify with `M115` when identity is uncertain.

## Mechanical design

- The current pen holder is a real calibrated tool, not a hypothetical future design.
- Preserve editable Fusion 360/CAD source as the canonical mechanical artifact. Printable STL/3MF and neutral STEP exports are derived artifacts, not substitutes for source CAD.
- Keep moving mass low, preserve cable clearance and document any change that affects the pen-tip coordinate envelope or Z calibration.
- A future modular tool interface may supersede the current holder, but do not discard the working pen baseline before the replacement is physically validated.

## Software

- Stock Marlin is the current firmware baseline.
- Kobra-specific profiles, limits, tool commands and live workflow belong here.
- Source conversion should eventually normalize SVG, text and raster images into a common plot-geometry representation before machine-specific fitting and G-code generation.
- Safety checks and calibration state must be explicit rather than inferred from printer model defaults.
