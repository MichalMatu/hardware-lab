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
- [x] 2026-09-29 map plot completed successfully with the current physical setup before the subsequent tuning request.

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
- [x] Adjacent start-touch continuous-path merge is on `main`.
- [ ] Land the tested end-touch/reverse continuous-path optimization currently represented by local commit `b358070` once GitHub push service is healthy; do not retry-loop around infrastructure errors.
- [ ] Add explicit filled-region strategies.
- [ ] Add raster/photo presets: outline, hatch/crosshatch and stipple.
- [ ] Add user-facing target-size/detail presets.

## Phase 4 — Durable live execution

- [x] Kobra-specific live runner exists as `kobra-live`.
- [x] Headless `T:0.00` policy and profile-driven Z feed are aligned with the committed profile on `main` (`7a3d510`).
- [x] Full maintenance verification passed (`33/33`, compileall, doctor, diff-check).
- [x] Read-only `SVG -> PREPARE -> kobra-live --validate-only -> READY_TO_PRINT` smoke test passed in **6.971 s**.
- [x] Kobra-specific execution remains in `hardware-lab`; `host-ops` stays generic.
- [ ] Add/standardize a small immutable `plot-job.json` manifest binding source hash, profile identity, G-code hash, bounds and validation state.
- [ ] Add explicit operator cancellation semantics if needed.
- [ ] Tie live preflight to calibration identity/freshness, not profile numbers alone.

## Phase 4A — 30-second chat-to-plot fast path

Target UX:

```text
user uploads image in ChatGPT
user: "drukuj"
-> one immutable pen-compatible SVG
-> PREPARE + hashes + safety preflight
-> PRINT as a separate bounded transaction
-> DRAWING_STARTED target: ~30 s from request under normal conditions
```

Requirements:

- [ ] Define one deterministic ChatGPT-upload -> single SVG transfer path; no gzip/base64 chunking.
- [ ] Add raster/photo conversion presets fast enough for interactive use.
- [ ] Keep ARTWORK, PREPARE and PRINT as separate responsibilities even if the user sees one command.
- [ ] Use the explicit `drukuj` request for the specific uploaded image as approval for that one prepared immutable job, after automatic safety checks succeed.
- [ ] Measure upload/vectorization/prepare/preflight/homing latency separately and keep the normal path inside the ~30 s start target.
- [ ] Never hide retries behind the single-command UX: one diagnosed corrective retry maximum per failed stage.

## Phase 5 — Mechanical evolution / additional tools

- [ ] Revisit quick-change tooling only when another real tool justifies it.
- [ ] Preserve an independent repeatable Z-reference strategy for future tools.
- [ ] Recalibrate the pen-tip envelope after any carriage/tool redesign.
- [ ] Define clay/paste delivery only after the pen workflow is stable.

## Phase 6 — Firmware decision

Revisit Klipper or another firmware only when a concrete requirement cannot be met cleanly by stock Marlin. Before any flash, identify the exact controller board/MCU and document rollback/recovery.