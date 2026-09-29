# Kobra 2 Neo pen plotter / motion platform

Anycubic Kobra 2 Neo converted into a reusable XY/Z pen-plotter motion platform. The current validated tool is a pen/marker holder; later tools may reuse the same project-level hardware and safety model.

## Current validated state — 2026-09-29

- Stock Marlin `bugfix-2.1.x` remains the firmware baseline.
- USB serial is CH340 at 115200 baud; identify the printer with `M115`, not by port name alone.
- The original printhead / hotend assembly is intentionally removed.
- The current plotter has **no hotend heater cartridge, no hotend thermistor and no printhead fans**.
- `M105` hotend `T:0.00` is therefore an **expected normal reading** for this machine and must not block pen plotting.
- Heater and extrusion commands remain forbidden in plot jobs. Do not use nozzle-temperature controls on the printer screen during pen plotting.
- A pen holder is installed and usable for bounded plotting motion.
- The cylindrical magnetic/proximity Z sensor is present and verified as `z_min`; it is the active Z homing reference for the pen setup. The rear physical button maps to `z_max`.
- Current pen-tip work envelope: `X=3..223`, `Y=36..230` mm.
- Normal plotting keeps an additional 5 mm internal margin.
- Current pen calibration: pen-up `Z=6.12`, pen-down `Z=2.97`.
- These coordinates are specific to the current pen, holder and paper placement and must be revalidated after mechanical changes.
- Verified start sequence: `G28 X Y` -> `G28 Z` -> `G0 Z6.12 F180`; the 2026-09-29 session also verified travel to `X90.69 Y134.14` after homing.
- Before XY homing, physically clear the complete Y-bed path. A printer power cable blocking rear travel caused a failed homing attempt on 2026-09-29; after moving the cable, the same homing sequence worked normally.
- First approved full artwork plot completed successfully on 2026-09-28: a 10 cm `MongooseLemur.svg` outline job completed all 7615 streamed commands in about 14 min 44 s and finished pen-up.
- Durable live execution is provided by the project-local `kobra-live` runner. Do not rebuild an ad-hoc serial streamer in Local Agent task payloads.

A firmware-reported `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or equivalent kill state **during an active transaction** remains a terminal failure. That is separate from the expected idle `T:0.00` caused by the intentionally absent hotend thermistor.

See `docs/CALIBRATION.md`, `docs/HARDWARE.md`, `docs/SAFETY.md`, `docs/WORKFLOW.md`, `docs/PREPARE_CLI.md`, `docs/GOLDEN_LIVE_FLOW.md` and `docs/THIRD_PARTY_PLOTTER_SOFTWARE.md` before extending the workflow or trying external plotter software.

## Repository boundary

This project is the canonical home for Kobra-specific hardware state, CAD, calibration, G-code policy, Marlin streaming policy and plotter workflow.

`host-ops` remains an external generic machine/serial capability layer. Its earlier `prototype/penplotter` remains useful historical/prototype evidence, but new Kobra-specific source conversion, fitting, safety policy and long-running plot protocol belong here rather than in `host-ops` core.

## Offline preparation CLI

The project contains the prepare-only `kobra-plot` CLI. Preparation never opens serial.

From `projects/kobra2-neo`:

```bash
uv sync
uv run kobra-plot doctor
uv run kobra-plot prepare samples/svg/curve-demo.svg
uv run kobra-plot text "MILEGO DNIA" -o /tmp/milego-dnia-job
```

A successful prepare job contains the copied source, normalized line SVG, path preview, bounded G-code and a JSON report. Preparation reports still mark execution as disabled because live execution is a separate explicit command. See `docs/PREPARE_CLI.md` for the artifact and safety contract.

Current V1 inputs:

- SVG, including curves flattened by the pinned `vpype` backend;
- UTF-8 text / literal text through vpype Hershey vector fonts;
- raster images and PDF are recognized but fail closed until explicit rendering strategies are implemented and tested.

## Live execution CLI

For an already prepared, inspected and explicitly approved job, use the dedicated runner rather than embedding Python serial code into a Local Agent task:

```bash
uv run kobra-live \
  samples/gcode/JOB.gcode \
  --report samples/gcode/JOB.report.json \
  --port /dev/cu.usbserial-130 \
  --expect-sha256 EXPECTED_SHA256
```

`kobra-live` performs full offline revalidation before opening the port, proves printer identity with `M115`, applies the current headless pen-plotter hardware profile, executes the explicit XY-home -> Z-home -> pen-up preamble, streams with acknowledgement after every command, emits Local Agent `[AGENT_PROGRESS]` checkpoints, waits for completion and finishes pen-up. See `docs/GOLDEN_LIVE_FLOW.md` for the authoritative fast path and evidence rules.

## Project layout

- `docs/` — current hardware state, calibration, safety, workflow, CLI contracts, incident reviews and golden live runbook.
- `cad/` — editable Fusion 360/CAD source and derived printable exports.
- `config/` — project-local machine/tool profiles.
- `src/kobra_plot.py` — offline source-to-job preparation.
- `src/kobra_live.py` — bounded execution of one already-approved prepared job.
- `tests/` — geometry, safety, offline integration and live-preflight tests; tests do not move hardware.
- `scripts/` — bounded operator utilities and project workflows.
- `samples/` — known-safe source files and generated dry-run examples.

## Direction

The canonical flow is:

`SVG / text / raster image -> normalized plot geometry -> fit/orientation -> bounded G-code -> dry-run/inspection -> explicit approval -> kobra-live -> structured progress/result evidence`

The preparation and execution boundaries stay separate. Raster/photo support may add outline, hatch/crosshatch and stipple renderers later without weakening the common fitting, G-code validation or live safety layer.
