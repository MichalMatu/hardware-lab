# Roadmap

## Phase 0 — Verified machine baseline

- [x] Working USB serial path identified and Marlin communication verified.
- [x] Firmware software endstops and `M119` sensor/endstop mapping recorded.
- [x] X/Y/Z motion and the cylindrical `z_min` reference verified.
- [x] Project isolated under `hardware-lab/projects/kobra2-neo`.

## Phase 1 — Working pen plotter baseline

- [x] Physical pen holder installed and usable.
- [x] Pen-up calibrated to Z=6.12.
- [x] Pen-down physically revalidated to Z=2.97.
- [x] Real pen-tip envelope measured: X=3..223, Y=36..230 mm.
- [x] Additional 5 mm normal plotting margin defined.
- [x] Project-local SVG/text preparation pipeline produces bounded G-code, preview and report.
- [x] First approved full artwork plot completed successfully on 2026-09-28.

## Phase 2 — Mechanical source

- [x] Canonical Fusion 360 pen-holder source stored under `cad/tools/pen/`.
- [ ] Add neutral STEP and printable STL/3MF exports where useful.
- [ ] Add a small set of installation photos/renders and stable measured tool offsets.
- [ ] Decide whether the current holder remains the reference tool or is superseded by a modular adapter.

## Phase 3 — Universal source pipeline

- [x] SVG curves/basic shapes -> normalized line geometry through pinned vpype.
- [x] Text -> vector strokes through Hershey fonts.
- [x] Geometry simplification, line ordering and pen-up travel preview.
- [x] Kobra orientation, fit and bounds remain a separate machine-specific stage.
- [ ] Add explicit strategies for filled regions where simple outlines are insufficient.
- [ ] Add raster/photo presets: outline, hatch/crosshatch and stipple.
- [ ] Add user-facing controls for physical target size and detail/simplification presets.

## Phase 4 — Durable live execution

- [x] Keep Kobra-specific preparation/policy in `hardware-lab`; keep `host-ops` generic.
- [x] Prove one complete live artwork using acknowledgement-driven serial streaming and final `M400`.
- [ ] Implement a permanent Kobra live streamer that accepts only a prepared, revalidated immutable job.
- [ ] Add explicit cancellation, progress and serial error/resend handling.
- [ ] Tie live preflight to the current calibration identity/freshness rather than profile values alone.

## Phase 5 — Mechanical evolution / additional tools

- [ ] Revisit a quick-change tool interface only when another real tool justifies it.
- [ ] Preserve a repeatable independent Z-reference strategy for future toolheads.
- [ ] Recalibrate the pen-tip envelope after any carriage/tool redesign.
- [ ] Define clay/paste delivery architecture only after the pen workflow is stable.

## Phase 6 — Firmware decision

Revisit Klipper or another firmware only when a concrete requirement cannot be met cleanly by stock Marlin. Before any flash, identify the exact controller board/MCU and document rollback/recovery.
