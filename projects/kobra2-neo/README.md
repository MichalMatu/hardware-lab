# Kobra 2 Neo pen plotter

Anycubic Kobra 2 Neo converted into a dedicated XY/Z pen plotter. The project owns the physical calibration, offline source preparation, G-code safety policy and bounded Marlin execution contract.

## Start here

For any continuation or handoff, read in this order:

1. `docs/HANDOFF.md`
2. `docs/WORKFLOW.md`
3. `config/kobra2_neo_pen.toml`
4. `docs/GOLDEN_LIVE_FLOW.md`
5. `docs/HARDWARE.md`, `docs/CALIBRATION.md`, `docs/SAFETY.md`

`docs/README.md` explains which documents are authoritative and which are historical only.

## Current physical state — 2026-09-29

- Stock Marlin `bugfix-2.1.x`.
- CH340 serial at 115200 baud; always prove printer identity with `M115`, never by port name alone.
- Original printhead/hotend assembly intentionally removed.
- No hotend heater cartridge, hotend thermistor or printhead fans.
- Pen/marker is the active tool.
- Cylindrical magnetic/proximity sensor is verified as `z_min` for Z homing; rear physical button maps to `z_max`.
- Idle hotend `T:0.00` is expected for the absent thermistor. A firmware-emitted halt/kill during an active transaction remains terminal.
- Hard pen-tip envelope: `X=3..223`, `Y=36..230` mm.
- Normal drawing envelope: `X=8..218`, `Y=41..225` mm.
- Current pen-down: `Z=2.97`.
- Current pen-up: `Z=4.97` — exactly 2.00 mm above pen-down.
- Current feeds: travel `6000`, draw `2400`, Z `360` mm/min.
- Orientation: Y flip, no XY swap, no X flip.
- Before XY homing, clear the full Y-bed path, especially the printer power cable.

The executable profile is `config/kobra2_neo_pen.toml`. If documentation or code disagrees with that profile, stop before physical execution and resolve the mismatch in a dedicated maintenance change.

## Canonical architecture

```text
ARTWORK
  one immutable source file + SHA-256
      -> PREPARE
  offline normalization / optimization / fit / validation
  -> output.gcode + preview + report + hashes
      -> REVIEW / APPROVAL
      -> PRINT
  immutable prepared job only; no code edits, no generation, no repo writes
```

One task has one responsibility. Do not combine code maintenance, artwork transport, preparation, Git synchronization and physical printing in one Local Agent task.

Generated artwork must be transferred as one normal file. Do not use hand-built gzip/base64 chunks or partial task payloads as an asset-transfer protocol.

## Offline preparation

From `projects/kobra2-neo`:

```bash
uv sync
uv run kobra-plot doctor
uv run kobra-plot prepare path/to/drawing.svg -o /tmp/drawing-job
```

Preparation never opens serial. It produces inspectable source/normalized geometry, preview, bounded G-code and a report. See `docs/PREPARE_CLI.md`.

## Live execution

`kobra-live` is the project-local bounded runner for one already prepared and approved job. It revalidates the immutable job, proves printer identity, homes, streams command-by-command with acknowledgements and requires a terminal pen-up result.

**Current handoff note:** the committed profile has the tuned values above, while the visible `main` implementation still has known runner/generator drift documented in `docs/HANDOFF.md`. Resolve that code-only maintenance gate and pass tests before the next unattended live job. Do not treat the existence of `kobra-live` as proof that the current revision is ready for physical execution.

## Project layout

- `docs/` — current operating contract, handoff and historical evidence.
- `config/` — machine/tool profile; executable calibration source of truth.
- `src/kobra_plot.py` — offline source-to-job preparation.
- `src/kobra_live.py` — bounded execution of one prepared job.
- `tests/` — offline geometry/safety/live-preflight tests; tests do not intentionally move hardware.
- `cad/` — pen-holder/tool CAD.
- `samples/` — small known inputs/examples, not an asset-transfer staging area.

## Repository boundary

Kobra-specific conversion, calibration, bounds, G-code policy and Marlin execution belong here. `host-ops` remains a generic host/device capability layer. Local Agent owns task scheduling/evidence; it is not the place to encode ad-hoc Kobra protocols.
