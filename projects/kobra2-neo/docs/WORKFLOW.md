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

The normalized geometry layer should represent drawable polylines/strokes independent of the Kobra. Text should become paths before machine fitting. Raster images will require an explicit rendering strategy such as contours, hatching, crosshatching, stippling or another line-based approximation; there is no generic concept of an inkjet-style filled pixel for a pen plotter.

## Current V1 implementation

The currently validated renderer lives in `MichalMatu/host-ops/prototype/penplotter` and implements:

`SVG -> strict parser -> orientation/fit -> bounded G-code`

Current V1 accepts simple SVG line geometry and path commands `M/L/H/V/Z`. It intentionally rejects unsupported curves, transforms and other constructs rather than silently approximating them.

This implementation remains outside `host-ops` core. Do not expand `host-ops` with Kobra-specific policy while this project is being consolidated.

## Current Kobra profile contract

- hard pen-tip envelope: X=3..223, Y=36..230 mm;
- normal internal margin: 5 mm;
- travel feed: 3000 mm/min;
- draw feed: 1200 mm/min;
- pen-up: `G0 Z6.12 F180`;
- pen-down: `G0 Z3.12 F180`;
- orientation: flip Y, no XY swap, no X flip;
- end sequence: `M400`;
- homing exists as an opt-in sequence only and must not be included in normal render/dry-run work.

## Required dry-run sequence

Before any live plotting job:

1. Convert/normalize the source without talking to the printer.
2. Render G-code without implicit homing.
3. Record input geometry statistics: polylines/strokes and point count.
4. Record generated XY bounding box.
5. Inspect every pen-up, pen-down, start/end and other profile command.
6. Verify there is no unintended `G28`.
7. Verify there are no heater or extrusion commands.
8. Verify all XY motion remains within the calibrated hard envelope.
9. Preview or otherwise inspect the complete plot path.
10. Only after operator approval open a live execution path.

## Live execution contract

There is no final plotter-specific streamer yet.

When live execution is introduced it must:

- remain project-specific or use only genuinely generic transport primitives from `host-ops`;
- handle Marlin acknowledgement/error responses explicitly;
- support bounded cancellation/error reporting;
- never silently insert homing, heating or extrusion;
- execute one physical operation at a time during bring-up and wait for operator confirmation between operations;
- never send a whole artwork until the corresponding dry-run has been explicitly approved.

## Next software design decision

Do not choose additional programs or libraries merely because the first complex SVG exceeds V1. First preserve the current machine/CAD baseline in this repository. Then evaluate source-conversion tools against the common normalized-geometry boundary, so SVG curves, text and raster strategies can be swapped without changing the Kobra safety layer.
