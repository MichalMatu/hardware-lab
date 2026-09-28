# Plotter workflow

## Goal

Build one safe workflow that can eventually accept vector art, text and raster images while keeping machine-specific fitting, calibration and execution bounded and inspectable.

Target architecture:

```text
SVG / text / raster image
    -> source-specific conversion
    -> normalized plot geometry
    -> simplify / optimize / ordering
    -> Kobra orientation + fit
    -> hard bounds validation
    -> G-code generation
    -> dry-run / preview / inspection
    -> explicit live streaming
```

The normalized geometry layer represents drawable polylines/strokes independent of the Kobra. Text becomes vector strokes before machine fitting. Raster images require an explicit rendering strategy such as contours, hatching, crosshatching or stippling; there is no generic inkjet-style filled pixel for a pen plotter.

## Current project-local prepare V1

`projects/kobra2-neo` now owns the prepare-only `kobra-plot` CLI:

`SVG / text -> vpype source conversion -> normalized line SVG -> Kobra orientation/fit -> bounded G-code -> preview/report`

V1 uses pinned `vpype==1.15.0` inside the project environment. SVG curves are linearized before the strict normalized-geometry parser. Text uses vpype's bundled Hershey vector fonts.

Raster and PDF extensions are detected but deliberately rejected in V1. They stay disabled until explicit line-rendering presets are implemented and tested.

The earlier `MichalMatu/host-ops/prototype/penplotter` remains prototype evidence. New Kobra-specific conversion, fitting and safety policy belongs in this project. `host-ops` remains the generic machine/serial capability layer for a future execution phase.

Most importantly, V1 contains no serial dependency and no `execute` command. Every successful report says `execution.allowed = false`.

See `PREPARE_CLI.md` for commands, artifacts and the validator contract.

## Current Kobra profile contract

- hard pen-tip envelope: X=3..223, Y=36..230 mm;
- normal internal margin: 5 mm;
- travel feed: 3000 mm/min;
- draw feed: 1200 mm/min;
- pen-up: `G0 Z6.12 F180`;
- pen-down: `G0 Z2.97 F180`;
- orientation: flip Y, no XY swap, no X flip;
- end sequence: `M400`;
- homing must not be included in normal render/dry-run work.

The profile is stored at `config/kobra2_neo_pen.toml`. Unknown or missing keys fail closed.

## Prepare output contract

A prepared job contains:

```text
source.<ext>
normalized.svg
preview.svg
output.gcode
report.json
```

`normalized.svg` is the source-independent boundary. `preview.svg` displays drawing strokes and pen-up travel separately. `report.json` captures source/backend/profile identity, hashes, geometry statistics, bounds, nominal feed-time estimate and safety state.

The generated G-code validator accepts only `G90`, `G0`, `G1` and `M400`. It rejects all other commands, extrusion parameter `E`, malformed/duplicate parameters, unexpected Z values and XY outside the hard pen-tip envelope.

## Required dry-run sequence

Before any future live plotting job:

1. Convert/normalize the source without talking to the printer.
2. Render G-code without implicit homing.
3. Record input geometry statistics: polylines/strokes and point count.
4. Record generated XY bounding box.
5. Inspect every pen-up, pen-down, start/end and other profile command.
6. Verify there is no unintended `G28` or relative mode.
7. Verify there are no heater or extrusion commands.
8. Verify all XY motion remains within the calibrated hard envelope.
9. Inspect `preview.svg` and the complete G-code.
10. Only after operator approval may a separate live execution path be considered.

## Live validation milestone — 2026-09-28

One complete approved artwork has now been executed successfully using the current calibration. The 10 cm `MongooseLemur.svg` outline job used 283 polylines / 7046 simplified points, machine bounds X=63.79..163.00 and Y=84.21..181.79, pen-up Z=6.12 and pen-down Z=2.97. A temporary Local Agent serial streamer waited for Marlin acknowledgement after every command, completed all 7615 commands in about 14 min 44 s, ended with `M400` and left the pen up.

This is hardware/workflow validation, not a permanent execution API. The project-local `kobra-plot` CLI remains prepare-only and has no serial dependency.

## Durable live execution contract

There is no final plotter-specific streamer yet. A future durable execution layer must:

- remain project-specific or use only genuinely generic transport primitives from `host-ops`;
- accept only a previously prepared and revalidated immutable job;
- handle Marlin acknowledgement/error responses explicitly;
- support bounded cancellation/error reporting;
- never silently insert homing, heating or extrusion;
- execute one physical operation at a time during bring-up and wait for operator confirmation between operations;
- never send a whole artwork until the corresponding dry-run has been explicitly approved.

## Next source-conversion decision

The normalized geometry and Kobra safety boundary are now explicit, and one complete live plot has validated the physical profile. The next source-conversion phase is raster/photo support plus explicit physical-size/detail controls. Evaluate outline, hatch/crosshatch and stipple as replaceable source renderers; do not weaken or bypass the common Kobra fitting/G-code validator to support them.
