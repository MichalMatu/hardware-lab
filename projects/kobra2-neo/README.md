# Kobra 2 Neo pen plotter / motion platform

Anycubic Kobra 2 Neo used as a reusable XY/Z motion platform. The current validated tool is a pen/marker holder; later tools may reuse the same project-level hardware and safety model.

## Current validated state — 2026-09-28

- Stock Marlin `bugfix-2.1.x` remains the firmware baseline.
- USB serial is CH340 at 115200 baud; identify the printer with `M115`, not by port name alone.
- A pen holder is installed and usable for bounded plotting motion.
- The cylindrical Z sensor is present and verified as `z_min`; the rear physical button maps to `z_max`.
- Current pen-tip work envelope: `X=3..223`, `Y=36..230` mm.
- Normal plotting keeps an additional 5 mm internal margin.
- Current pen calibration: pen-up `Z=6.12`, pen-down `Z=3.12`.
- These coordinates are specific to the current pen, holder and paper placement and must be revalidated after mechanical changes.

See `docs/CALIBRATION.md`, `docs/HARDWARE.md`, `docs/SAFETY.md`, `docs/WORKFLOW.md` and `docs/PREPARE_CLI.md` before live motion.

## Repository boundary

This project is the canonical home for Kobra-specific hardware state, CAD, calibration, G-code policy and plotter workflow.

`host-ops` remains an external generic machine/serial capability layer. Its earlier `prototype/penplotter` remains useful historical/prototype evidence, but new Kobra-specific source conversion, fitting and safety policy belongs here rather than in `host-ops` core.

## Offline preparation CLI

The project now contains the prepare-only `kobra-plot` CLI. V1 intentionally has no serial transport and no `execute` command.

From `projects/kobra2-neo`:

```bash
uv sync
uv run kobra-plot doctor
uv run kobra-plot prepare samples/svg/curve-demo.svg
uv run kobra-plot text "MILEGO DNIA" -o /tmp/milego-dnia-job
```

A successful prepare job contains the copied source, normalized line SVG, path preview, bounded G-code and a JSON report. The report always marks execution as disabled. See `docs/PREPARE_CLI.md` for the artifact and safety contract.

Current V1 inputs:

- SVG, including curves flattened by the pinned `vpype` backend;
- UTF-8 text / literal text through vpype Hershey vector fonts;
- raster images and PDF are recognized but fail closed until explicit rendering strategies are implemented and tested.

## Project layout

- `docs/` — current hardware state, calibration, safety, workflow, CLI contract, roadmap and historical checkpoints.
- `cad/` — editable Fusion 360/CAD source and derived printable exports.
- `config/` — project-local machine/tool profiles.
- `src/` — Kobra-specific offline preparation code; future live execution remains separate.
- `tests/` — geometry, safety and offline integration tests.
- `scripts/` — future bounded operator utilities and project workflows.
- `samples/` — known-safe source files and generated dry-run examples.

## Direction

The long-term input flow is:

`SVG / text / raster image -> normalized plot geometry -> fit/orientation -> bounded G-code -> dry-run/inspection -> explicit live execution`

The current implementation establishes the source-to-job half of that boundary. The next conversion work is to add explicit raster/photo presets such as outline, hatch/crosshatch and stipple without changing the Kobra fitting/safety layer. Live serial execution remains a separate later phase.
