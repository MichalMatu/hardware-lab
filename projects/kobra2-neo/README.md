# Kobra 2 Neo modular motion platform

Anycubic Kobra 2 Neo converted from a stock FDM printer into a reusable three-axis motion platform with interchangeable toolheads.

## Current direction

- Stock print head and original Z probe are removed.
- Stock Marlin remains in use while it satisfies the motion requirements.
- USB serial control is already validated through CH340 at 115200 baud.
- The first tool is a pen/marker module.
- The mechanical interface must remain suitable for later clay/paste and other toolheads.
- `host-ops` is an external generic execution/tooling layer; Kobra-specific behavior lives here.

## Project layout

- `docs/` — hardware state, safety contract, roadmap and checkpoints.
- `cad/` — editable carriage/interface and toolhead CAD plus derived printable exports.
- `src/` — device-specific host-side motion/protocol/calibration code when it becomes necessary.
- `scripts/` — bounded operator utilities and project workflows.
- `samples/` — known-safe sample G-code/SVG inputs.

## Immediate next step

Measure and document the bare X-carriage geometry, then define a repeatable common adapter datum before designing the first pen holder.
