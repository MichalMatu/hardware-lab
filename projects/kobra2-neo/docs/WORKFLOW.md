# Plotter workflow

## Goal

Keep source conversion, machine fitting, dry-run inspection and physical execution as explicit boundaries while making an already-approved live plot fast and repeatable.

Canonical architecture:

```text
SVG / text / raster image
    -> source-specific conversion
    -> normalized plot geometry
    -> simplify / optimize / ordering
    -> Kobra orientation + fit
    -> hard bounds validation
    -> G-code generation
    -> dry-run / preview / inspection
    -> explicit operator approval
    -> kobra-live revalidation
    -> M115 identity
    -> headless pen-plotter profile check
    -> XY home -> Z home -> pen up
    -> acknowledged artwork stream
    -> M400 -> final pen up
    -> structured result evidence
```

The normalized geometry layer represents drawable polylines/strokes independent of the Kobra. Text becomes vector strokes before machine fitting. Raster images require an explicit rendering strategy such as contours, hatching, crosshatching or stippling.

## Current hardware mode

The machine is a dedicated pen plotter, not a complete stock 3D-printer toolhead configuration:

- original printhead / hotend removed;
- no hotend heater cartridge;
- no hotend thermistor;
- no printhead fans;
- pen/marker is the active tool;
- cylindrical magnetic/proximity sensor is the verified `z_min` used for Z homing;
- `M105` hotend `T:0.00` is expected normal state for the intentionally absent thermistor.

Do not add an `M105` room-temperature gate to this workflow and do not require the hotend thermistor to be reconnected. Heater commands remain forbidden. If Marlin itself enters a kill/halt state during a transaction, that is still a terminal execution failure.

## Project-local prepare V1

`projects/kobra2-neo` owns the prepare-only `kobra-plot` CLI:

`SVG / text -> vpype source conversion -> normalized line SVG -> Kobra orientation/fit -> bounded G-code -> preview/report`

V1 uses pinned `vpype==1.15.0`. Raster and PDF extensions are detected but deliberately rejected until explicit line-rendering presets are implemented and tested.

Preparation never opens serial. Every prepare report keeps execution disabled; physical execution is a separate operator-approved command.

## Repository boundary

- `hardware-lab/projects/kobra2-neo` owns Kobra-specific conversion, calibration, limits, hardware-profile policy, Marlin protocol policy and long-running plot streaming.
- `host-ops` owns generic host/device capabilities such as macOS serial enumeration and bounded raw serial transactions. It is not the Kobra protocol executor.
- Local Agent owns repository binding, task scheduling/watchdogs and task/run/result evidence.
- A binary available in one Local Agent binding must never be assumed available in another worker PATH.

## Current Kobra profile contract

- hard pen-tip envelope: X=3..223, Y=36..230 mm;
- normal internal margin: 5 mm;
- travel feed: 3000 mm/min;
- draw feed: 1200 mm/min;
- pen-up: `G0 Z6.12 F180`;
- pen-down: `G0 Z2.97 F180`;
- orientation: flip Y, no XY swap, no X flip;
- end sequence: `M400`;
- homing is forbidden in prepare-generated artwork and belongs only to the approved live preamble;
- hotend/thermistor presence must not be assumed for the pen profile.

The profile is stored at `config/kobra2_neo_pen.toml`. Unknown or missing safety-critical keys fail closed.

## Prepare output contract

A prepared job contains source material, normalized geometry, preview, bounded G-code and a report. The generated artwork validator accepts only `G90`, `G0`, `G1` and `M400`. It rejects all other commands, extrusion parameter `E`, malformed/duplicate parameters, unexpected Z values and XY outside the calibrated envelope.

## Required dry-run sequence

Before live plotting:

1. Convert/normalize without talking to the printer.
2. Render G-code without implicit homing.
3. Record geometry statistics and generated XY bounds.
4. Inspect pen-up/down and start/end commands.
5. Verify there is no unintended `G28`, relative mode, heater or extrusion command.
6. Verify all XY motion remains within the calibrated envelope.
7. Inspect `preview.svg` and the complete G-code.
8. Verify the physical XY path is clear, especially the rear Y-bed path and printer power cable.
9. Obtain explicit approval for the complete physical transaction.

## Durable live execution

Use `kobra-live`; do not create another temporary serial streamer in a Local Agent task.

Example:

```bash
uv run kobra-live \
  samples/gcode/JOB.gcode \
  --report samples/gcode/JOB.report.json \
  --port /dev/cu.usbserial-130 \
  --expect-sha256 EXPECTED_SHA256
```

Before opening serial the runner revalidates the report, command whitelist, normal plotting envelope, calibrated Z values and optional SHA-256 pin. It then:

1. opens exactly the explicit serial path at 115200;
2. requires `M115` evidence identifying Anycubic Kobra;
3. uses the current headless pen-plotter hardware contract; the intentionally absent hotend thermistor / `T:0.00` is not a motion blocker;
4. sends `G28 X Y` and waits for `ok`;
5. sends `G28 Z` and waits for `ok`;
6. selects absolute mode and raises the pen;
7. streams the already-approved artwork command by command, waiting for Marlin acknowledgement after every command;
8. treats firmware errors, resend requests and acknowledgement timeouts as terminal failures rather than blind retry opportunities;
9. treats a firmware-emitted `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or equivalent Marlin kill state during execution as terminal, with final pen state unknown unless a later acknowledgement proves otherwise;
10. after the artwork `M400`, raises the pen again and waits for a final `M400`;
11. emits a terminal pen-up result only after that final safe state is acknowledged.

For a non-fatal streaming/transport failure after successful Z homing, the runner makes one bounded best-effort pen-up recovery attempt and reports whether it was acknowledged. After a Marlin kill state it does not pretend that additional motion is reliable.

## Structured progress and evidence

Long physical tasks must emit Local Agent native markers:

```text
[AGENT_PROGRESS] {"stage_name":"kobra-live",...}
```

The runner reports `PREFLIGHT_OK`, `PRINTER_IDENTIFIED`, `HOMING_XY_OK`, `HOMING_Z_OK`, `PEN_UP_OK`, `DRAWING_STARTED`, periodic drawing progress and `COMPLETE_PEN_UP`. A fatal Marlin safety stop reports `FIRMWARE_HALTED_FINAL_PEN_STATE_UNKNOWN`.

Never infer a physical stage from process liveness or `seconds_since_output`. Say a stage passed only when its structured progress/result evidence exists.

## Fast-path transport rule

A new chat does not require a new host-ops probe when the host/cabling session is unchanged and the serial path is known. The live runner performs its own `M115` identity check before motion. Use host-ops discovery/probe only if the port is unknown, changed, ambiguous or the runner's identity check fails.

Never open a host-ops serial probe concurrently with an active live task on the same printer.

## Live validation milestones

- 2026-09-28: 10 cm `MongooseLemur.svg` outline completed all 7615 acknowledged commands in about 14 min 44 s and finished pen-up, validating the physical pen profile and acknowledgement-driven streaming model.
- 2026-09-28: the slow-start incident exposed missing durable execution/orchestration. `kobra-live`, structured progress and the golden runbook were added as corrective actions.
- 2026-09-28: `shaft-50x20-showcase` later reached command 3300/4345 before stock Marlin halted on `MINTEMP` for E0. That run finished failed after 801.199 s with no confirmed final pen-up. The corrected interpretation is that a live firmware kill is terminal; it does not mean the intentionally removed thermistor must be present or that `T:0.00` should block the current pen-only configuration.
- 2026-09-29: `G28 X Y -> G28 Z -> G0 Z6.12 F180 -> G0 X90.69 Y134.14 F3000` was re-verified. An initial failed Y homing attempt was traced to the printer power cable physically blocking bed travel; after clearing the cable, the same sequence worked normally.

See `GOLDEN_LIVE_FLOW.md` for the authoritative operator flow and `HARDWARE.md` for the canonical physical configuration.
