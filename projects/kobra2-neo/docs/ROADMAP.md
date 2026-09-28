# Roadmap

## Phase 0 — Verified machine baseline

- [x] Working USB serial path identified.
- [x] Stock Marlin communication verified.
- [x] Firmware software endstops recorded.
- [x] X/Y/Z motion verified.
- [x] `M119` sensor/endstop mapping verified.
- [x] Current cylindrical Z-reference path verified with `G28 Z`.
- [x] Project isolated under `hardware-lab/projects/kobra2-neo`.

## Phase 1 — Working pen plotter baseline

- [x] Physical pen holder installed and usable.
- [x] Pen-up calibrated to Z=6.12.
- [x] Pen-down calibrated to Z=3.12.
- [x] Real pen-tip envelope measured: X=3..223, Y=36..230 mm.
- [x] Additional 5 mm normal plotting margin defined.
- [x] SVG-to-bounded-G-code V1 validated externally in `host-ops/prototype/penplotter`.
- [ ] Complete the first approved artwork plot using the dry-run/live contract.

## Phase 2 — Consolidate mechanical source

- [x] Add the existing Fusion 360 pen-holder source under `cad/tools/pen/`.
- [ ] Add neutral STEP export and printable STL/3MF where useful.
- [ ] Add a small set of photos/renders showing installed orientation and critical clearances.
- [ ] Document pen diameter/clamping range, compliance and tool-to-carriage offsets if they are stable and measured.
- [ ] Decide whether the current holder becomes the reference tool or is superseded by a common modular adapter.

## Phase 3 — Universal source pipeline

Define one input architecture whose machine-independent output is normalized plot geometry.

- [ ] Complex SVG: curves, transforms and filled regions through explicit flattening/vector strategies.
- [ ] Text: font/text -> paths/strokes -> normalized geometry.
- [ ] Raster images/photos: selectable contour, hatch/crosshatch, stipple or other pen-compatible strategies.
- [ ] Geometry simplification and pen-up travel ordering/optimization.
- [ ] Preview that distinguishes drawing motion from travel motion.
- [ ] Retain Kobra fit/orientation/bounds as a separate machine-specific stage.

Do not commit to an external program/library until its role at this boundary is clear.

## Phase 4 — Project-local execution workflow

- [ ] Decide where the plotter renderer should permanently live after the architecture is proven.
- [ ] Keep `host-ops` generic; reuse it only for genuinely generic serial/device capabilities.
- [ ] Add a plotter-specific Marlin streamer with acknowledgement/error handling.
- [ ] Add preflight validation against the current calibration state.
- [ ] Add known-safe sample inputs and expected dry-run summaries.

## Phase 5 — Mechanical evolution / independent reference

- [ ] Revisit a common quick-change tool interface if more tools justify it.
- [ ] Preserve or redesign an independent, repeatable Z-reference strategy for future toolheads.
- [ ] Recalibrate the full pen-tip envelope after any carriage/tool redesign.

## Phase 6 — Additional tools

- [ ] Define clay/paste delivery architecture only after the pen workflow is stable.
- [ ] Prefer remote reservoir + lightweight carriage nozzle where practical.
- [ ] Decide whether a dedicated actuator/controller is required.

## Phase 7 — Firmware decision

Revisit Klipper or another firmware only when a concrete requirement cannot be met cleanly by stock Marlin. Before any flash, identify the exact controller board/MCU and document rollback/recovery.
