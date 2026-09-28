# Kobra 2 Neo pen plotter / motion platform

Anycubic Kobra 2 Neo used as a reusable XY/Z motion platform. The current validated tool is a pen/marker holder; later tools may reuse the same project-level hardware and safety model.

## Current validated state — 2026-09-28

- Stock Marlin `bugfix-2.1.x` remains the firmware baseline.
- USB serial is CH340 at 115200 baud; identify the printer with `M115`, not by port name alone.
- A pen holder is installed and usable for bounded plotting motion.
- The cylindrical Z sensor is present and verified as `z_min`; the rear physical button maps to `z_max`.
- Current pen-tip work envelope: `X=3..223`, `Y=36..230` mm.
- Normal plotting keeps an additional 5 mm internal margin.
- Current pen calibration: pen-up `Z=6.12`, pen-down `Z=3.12`.
- These coordinates are specific to the current pen, holder and paper placement and must be revalidated after mechanical changes.

See `docs/CALIBRATION.md`, `docs/HARDWARE.md`, `docs/SAFETY.md` and `docs/WORKFLOW.md` before live motion.

## Repository boundary

This project is the canonical home for Kobra-specific hardware state, CAD, calibration, G-code policy and plotter workflow.

`host-ops` remains an external generic machine/serial capability layer. The currently validated SVG-to-bounded-G-code implementation lives temporarily in `MichalMatu/host-ops/prototype/penplotter`; do not move more Kobra-specific policy into `host-ops` core.

## Project layout

- `docs/` — current hardware state, calibration, safety, workflow, roadmap and historical checkpoints.
- `cad/` — editable Fusion 360/CAD source and derived printable exports.
- `src/` — future Kobra-specific host-side software when justified.
- `scripts/` — future bounded operator utilities and project workflows.
- `samples/` — future known-safe source files and generated dry-run artifacts.

## Direction

The long-term input flow is:

`SVG / text / raster image -> normalized plot geometry -> fit/orientation -> bounded G-code -> dry-run/inspection -> explicit live execution`

The current hardware, calibration and Fusion 360 pen-holder source are now consolidated here. The next software step is to evaluate source-conversion options for complex SVG, text and raster images against the normalized-geometry boundary in `docs/WORKFLOW.md`, without moving Kobra-specific policy into `host-ops` core.
