# `kobra-plot` offline preparation CLI

`kobra-plot` prepares inspectable pen-plotter jobs for the current Kobra 2 Neo setup. V1 is deliberately offline-only: it has no serial dependency and no `execute` command.

## Safety boundary

`prepare` may read source files and create local artifacts. It must not:

- discover or open a serial port;
- home the printer;
- send G-code;
- heat or extrude;
- decide that a prepared job is approved for execution.

Every V1 report therefore contains:

```json
"execution": {
  "allowed": false,
  "reason": "prepare-only V1 has no live transport or execute command"
}
```

## Setup

From `projects/kobra2-neo`:

```bash
uv sync
uv run kobra-plot doctor
```

The project pins `vpype==1.15.0`; it does not require a global `vpype` installation.

## SVG

Prepare an SVG without touching the printer:

```bash
uv run kobra-plot prepare samples/svg/curve-demo.svg
```

The default output is `<source-name>.kobra-job/`. Use an explicit directory when useful:

```bash
uv run kobra-plot prepare drawing.svg -o /tmp/drawing-job
```

The SVG backend uses vpype to linearize SVG curves, simplify geometry and order strokes before the Kobra-specific fit/safety stage. The included `curve-demo.svg` deliberately contains Bézier `C` and `S` commands so the end-to-end test proves that curved source SVG is flattened before the strict normalized-geometry parser sees it.

## Text

Literal text can be converted with vpype's bundled Hershey vector fonts:

```bash
uv run kobra-plot text "MILEGO DNIA" -o /tmp/milego-dnia-job
```

A UTF-8 `.txt` file is also accepted by `prepare`.

## Raster and PDF inputs

V1 recognizes common raster extensions (`.png`, `.jpg`, `.jpeg`, `.webp`, `.bmp`, `.tif`, `.tiff`) and PDF, but rejects them intentionally. A future phase must choose an explicit pen-compatible rendering strategy such as outline, hatch/crosshatch or stipple and test it before these inputs are enabled.

## Job artifacts

A successful job directory contains:

```text
source.<ext>
normalized.svg
preview.svg
output.gcode
report.json
```

`normalized.svg` is the source-independent line geometry after source conversion/optimization. `preview.svg` shows drawing paths in black and pen-up travel as dashed gray lines. `output.gcode` is generated from the current Kobra profile. `report.json` records source/profile/backend identity, geometry statistics, bounds, nominal motion time, hashes and safety state.

The nominal time estimate is geometric feed-time only. It does not model firmware acceleration, planner overhead or the initial XY move to the first stroke.

## G-code contract

The V1 validator only accepts:

- `G90` — absolute positioning;
- `G0` — bounded travel/Z moves;
- `G1` — bounded drawing moves;
- `M400` — planner completion at the end.

It rejects every other command, extrusion parameter `E`, malformed/duplicate parameters, XY outside the measured hard pen-tip envelope, and unexpected Z values. Homing (`G28`), relative mode (`G91`), heaters and extrusion are therefore rejected by construction.

The profile lives at `config/kobra2_neo_pen.toml` and contains the current measured calibration. Unknown profile keys fail closed.

## Review before future live execution

A prepared job is not permission to run it. Before a future execution layer may accept a job, review at least:

1. `preview.svg`;
2. `report.json` bounds, stroke/point count and distances;
3. the complete `output.gcode`;
4. profile/calibration validity for the current pen, holder and paper;
5. absence of homing, heater and extrusion commands.

Live transport remains a separate future step and should reuse only generic machine/serial capabilities from `host-ops`.
