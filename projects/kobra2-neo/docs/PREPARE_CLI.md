# `kobra-plot` offline preparation CLI

`kobra-plot` prepares inspectable pen-plotter jobs for the current Kobra 2 Neo setup. V1 is deliberately offline-only: it has no serial dependency and no `execute` command.

## Safety boundary

`prepare` may read source files and create local artifacts. It must not discover/open a serial port, home the printer, send G-code, heat/extrude, or decide that a prepared job is approved for execution.

Every V1 report contains `execution.allowed = false`.

## Setup

From `projects/kobra2-neo`:

```bash
uv sync
uv run kobra-plot doctor
```

The project pins `vpype==1.15.0`; no global vpype installation is required. External vpype calls have a 300-second timeout.

## SVG

```bash
uv run kobra-plot prepare samples/svg/curve-demo.svg
uv run kobra-plot prepare drawing.svg -o /tmp/drawing-job
```

vpype linearizes SVG curves, simplifies geometry and orders strokes before the Kobra-specific fit/safety stage. The included `curve-demo.svg` deliberately contains Bézier `C` and `S` commands so end-to-end tests prove that curves are flattened before the strict normalized-geometry parser sees them.

V1 accepts source SVG elements that map directly to plot geometry: `svg`, `g`, `path`, `line`, `polyline`, `polygon`, `rect`, `circle`, and `ellipse`. Unsupported elements such as embedded text/images, `<use>`, clipping/filter constructs and other silently-discardable content fail closed. External references are forbidden.

Limits:

- source size: 20 MiB;
- XML elements: 10,000;
- nesting depth: 128;
- normalized polylines: 10,000;
- normalized points: 100,000.

After normalization, unexpected element types and transform attributes fail closed.

## Text

```bash
uv run kobra-plot text "MILEGO DNIA" -o /tmp/milego-dnia-job
```

A UTF-8 `.txt` file is also accepted by `prepare`. Text uses vpype's bundled Hershey vector fonts.

## Raster and PDF inputs

V1 recognizes common raster extensions and PDF but rejects them intentionally. A later phase must implement and test explicit pen-compatible renderers such as outline, hatch/crosshatch or stipple.

## Job artifacts

A successful job directory contains:

```text
.kobra-plot-job
source.<ext>
normalized.svg
preview.svg
output.gcode
report.json
```

The marker makes `--force` safe: only a directory previously created by `kobra-plot` may be recursively replaced. An arbitrary existing directory is never removed by `--force`.

`normalized.svg` is the source-independent line geometry. `preview.svg` shows drawing paths in black and pen-up travel as dashed gray lines. `report.json` records source/profile/backend identity, source and artifact hashes, geometry statistics, bounds, nominal feed-time estimate and safety state.

The time estimate is geometric feed-time only; it does not model firmware acceleration, planner overhead or the initial XY move to the first stroke.

## G-code contract

The validator accepts only this generated command vocabulary:

- one leading `G90`;
- `G0 X... Y... F...` only while the pen is known up, using the configured travel feed;
- `G0 Z... F...` only for the exact configured pen-up/pen-down Z values and Z feed;
- `G1 X... Y... F...` only while the pen is known down, using the configured draw feed;
- one trailing `M400`.

Every XY move must include both X and Y and remain inside the normal drawing envelope (`X=8..218`, `Y=41..225` for the current profile). The validator rejects every other command, `E`, malformed/duplicate parameters, unexpected feeds, unsafe pen-state transitions, unexpected Z values and motion outside that envelope. `G28`, `G91`, heater commands and extrusion therefore fail closed.

The profile lives at `config/kobra2_neo_pen.toml`. Unknown/missing keys and invalid types fail closed; prepare V1 also requires the end sequence to be exactly `M400`.

## Review before future live execution

A prepared job is not permission to run it. Before any future execution layer accepts a job, review at least `preview.svg`, report bounds/stroke counts/distances, the complete G-code, and whether the current physical calibration still matches the pen/holder/paper.

Live transport remains a separate future step and should reuse only generic machine/serial capabilities from `host-ops`.
