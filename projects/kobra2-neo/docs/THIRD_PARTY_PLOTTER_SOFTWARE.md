# Third-party plotter software candidates

Status: evaluation backlog. None of these tools is a validated live-control path for this Kobra. The preferred role for third-party art software is **source generation/vectorization only**, followed by the canonical project-local PREPARE -> REVIEW -> PRINT flow.

Current machine values come only from `config/kobra2_neo_pen.toml`. At the 2026-09-29 handoff:

```text
pen up: Z=4.97
pen down: Z=2.97
normal drawing envelope: X=8..218, Y=41..225 mm
travel feed: 6000 mm/min
draw feed: 2400 mm/min
Z feed: 360 mm/min
```

Do not copy old values from third-party profiles or historical project notes.

## 1. GCodeScribe

Repository: https://github.com/Friedjof/gcodescribe

Potential value:

- browser-based pen-plotter studio;
- PDF/SVG/raster/Office import;
- vpype-based conversion and preview;
- profile/job fingerprints and safety concepts.

Evaluation rule: prefer using it to produce a normal SVG first. Any direct G-code or serial path remains untrusted until independently validated against this project's current profile and safety policy.

## 2. OmniPlot (`rsp-pen-plotter`)

Repository: https://github.com/glloq/rsp-pen-plotter

Potential value:

- broad bitmap/SVG/PDF/CAD/text conversion;
- potrace, stippling, hatching and halftone algorithms;
- preview, bounds and path optimization;
- machine-profile architecture.

Evaluation rule: use conversion output as a single source artifact and pass it through `kobra-plot`; do not replace the canonical Kobra profile with an external YAML profile.

## 3. Plottter

Repository: https://github.com/pywkt/plottter

Potential value:

- strong raster-to-vector algorithms;
- hatching, stippling, contour/edge methods and generative tools;
- SVG export and path optimization.

Preferred use: `image -> optimized SVG`, then canonical project-local PREPARE. This is the least coupled to live printer control.

## Evaluation order

1. Plottter for raster/image -> SVG.
2. GCodeScribe for conversion/layout ideas.
3. OmniPlot for converter/profile architecture.

## Safety / workflow boundary

Third-party software does not bypass `WORKFLOW.md`.

A candidate output must become one normal immutable source file. Do not transport it with manual base64 chunks. Then run project-local PREPARE, inspect the actual preview/report/G-code, obtain approval and only then use the project PRINT contract.

Do not use third-party direct serial control on this machine merely because the software supports Marlin.