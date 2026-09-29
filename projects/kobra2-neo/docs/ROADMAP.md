# Roadmap

## Phase 0 — Verified machine baseline

- [x] Working USB serial path identified and Marlin communication verified.
- [x] Firmware software endstops and `M119` sensor/endstop mapping recorded.
- [x] X/Y/Z motion and cylindrical `z_min` reference verified.
- [x] Project isolated under `hardware-lab/projects/kobra2-neo`.

## Phase 1 — Working pen plotter baseline

- [x] Pen holder installed and usable.
- [x] Pen-down calibrated to `Z=2.97`.
- [x] Pen-up tuned to `Z=4.97` (2.00 mm lift).
- [x] Pen-tip envelope measured: X=3..223, Y=36..230 mm.
- [x] 5 mm normal plotting margin defined.
- [x] Current feeds set to travel 6000, draw 2400, Z 360 mm/min.
- [x] First approved full artwork completed successfully.
- [x] 2026-09-29 map plot completed successfully with current physical setup before the subsequent tuning request.

## Phase 2 — Mechanical source

- [x] Canonical Fusion 360 pen-holder source stored under `cad/tools/pen/`.
- [ ] Add neutral STEP and printable STL/3MF exports where useful.
- [ ] Add stable installation photos/renders and measured offsets.
- [ ] Decide whether the current holder remains the reference tool or is superseded by a modular adapter.

## Phase 3 — Universal source pipeline

- [x] SVG curves/basic shapes -> normalized line geometry through pinned vpype.
- [x] Text -> vector strokes.
- [x] Kobra orientation, fit and bounds are a machine-specific stage.
- [x] Preview/report/G-code preparation exists offline.
- [ ] Merge tested contiguous-path optimization into `main` and keep conservative no-gap semantics.
- [ ] Add explicit filled-region strategies.
- [ ] Add raster/photo presets: outline, hatch/crosshatch and stipple.
- [ ] Add user-facing target-size/detail presets.

## Phase 4 — Durable live execution

- [x] Kobra-specific live runner exists as `kobra-live`.
- [x] Acknowledgement-driven streaming and terminal progress markers are implemented conceptually and have physical evidence from prior runs.
- [x] Kobra-specific execution remains in `hardware-lab`; `host-ops` stays generic.
- [ ] Resolve the 2026-09-29 `main` drift documented in `HANDOFF.md`: headless `T:0.00` policy, profile-driven Z feed and tested path merge must be aligned with the committed profile/tests.
- [ ] Add/standardize a small immutable `plot-job.json` manifest binding source hash, profile identity, G-code hash, bounds and validation state.
- [ ] Add explicit operator cancellation semantics if needed.
- [ ] Tie live preflight to calibration identity/freshness, not profile numbers alone.

The next physical regression job must not be chosen until the code-maintenance gate above is green. Once green, use the exact ARTWORK -> PREPARE -> REVIEW -> PRINT flow in `WORKFLOW.md` rather than a combined task.

## Phase 5 — Mechanical evolution / additional tools

- [ ] Revisit quick-change tooling only when another real tool justifies it.
- [ ] Preserve an independent repeatable Z-reference strategy for future tools.
- [ ] Recalibrate the pen-tip envelope after any carriage/tool redesign.
- [ ] Define clay/paste delivery only after the pen workflow is stable.

## Phase 6 — Firmware decision

Revisit Klipper or another firmware only when a concrete requirement cannot be met cleanly by stock Marlin. Before any flash, identify the exact controller board/MCU and document rollback/recovery.