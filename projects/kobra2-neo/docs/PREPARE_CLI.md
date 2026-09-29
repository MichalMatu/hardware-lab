# `kobra-plot` offline preparation CLI

`kobra-plot` converts one source file into one inspectable prepared pen-plotter job. PREPARE is deliberately offline-only and must stay independent from live serial execution.

## Boundary

PREPARE may:

- read one normal source file;
- normalize/vectorize supported input;
- simplify/order geometry;
- apply current Kobra orientation and fit;
- generate preview, report and G-code;
- validate command vocabulary, feeds, Z values and XY bounds;
- compute artifact hashes/statistics.

PREPARE must not:

- open/discover a serial port;
- home or move the printer;
- heat/extrude;
- edit project code;
- commit/push as part of normal job preparation;
- decide that the job is physically approved.

## Input transport rule

The input is one normal file. For generated artwork, finish conversion/vectorization first and transfer/store the result as one `.svg` (or another supported single file).

Do **not** use manually split gzip/base64 chunks, a large task JSON string or a sequence of partial GitHub files as an artwork-transfer mechanism. That approach caused repeated failed preparation attempts on 2026-09-29 and is explicitly retired.

If the source cannot be transferred as one file through the available connector/workspace path, stop before PREPARE and choose a proper file-transfer mechanism.

## Setup

From `projects/kobra2-neo`:

```bash
uv sync
uv run kobra-plot doctor
```

The project pins its source-conversion dependencies. Do not rely on a different worker having a global `vpype` binary.

## SVG

```bash
uv run kobra-plot prepare path/to/drawing.svg -o /tmp/drawing-job
```

The SVG path is linearized/normalized before the strict Kobra geometry stage. Unsupported or externally referenced SVG content must fail closed rather than disappear silently.

## Text

```bash
uv run kobra-plot text "MILEGO DNIA" -o /tmp/milego-dnia-job
```

UTF-8 text uses vector stroke fonts before machine fitting.

## Raster / generated images

The current core prepare CLI does not treat arbitrary raster art as magically printable. Raster/photo input requires an explicit conversion strategy (for example contour, hatch/crosshatch or stipple) that outputs normal vector/plot geometry first.

For ChatGPT-generated artwork the preferred pipeline is:

```text
generated image -> deliberate vectorization/simplification -> one final SVG -> PREPARE
```

Do not combine vectorizer development with the same task that is expected to prepare or print a physical job.

## Job artifacts

A successful prepared job contains the equivalent of:

```text
source.<ext>
normalized.svg
preview.svg
output.gcode
report.json
```

`report.json` records enough identity/safety information to bind the reviewed source/profile/G-code. Hashes must be preserved through REVIEW and PRINT.

## G-code contract

Generated artwork is bounded motion only. The current profile is `config/kobra2_neo_pen.toml`.

At the 2026-09-29 handoff it defines:

```text
travel feed = 6000 mm/min
draw feed   = 2400 mm/min
Z feed      = 360 mm/min
pen up      = Z4.97
pen down    = Z2.97
normal XY   = X8..218, Y41..225 mm
end         = M400
```

Prepared artwork must not contain homing, heaters, extrusion or unexpected relative-mode behavior. Homing is a live PRINT preamble concern.

## Path optimization rule

Stroke ordering may reduce pen-up travel. Consecutive paths may be merged without lifting only when their endpoints actually touch within a tested conservative tolerance. Never draw an unintended connector across a real gap merely to reduce Z cycles.

The 2026-09-29 handoff records that this optimization was tested in a local-only commit but was not yet merged to `main`; resolve that code-maintenance gate before relying on it.

## Success / failure

PREPARE ends in either:

```text
READY_TO_PRINT
```

or a concrete failure. A failed PREPARE does not trigger PRINT and must not automatically spawn a chain of variant preparation tasks.

Review `preview.svg`, report and G-code before physical approval. See `WORKFLOW.md` for the full stage contract.