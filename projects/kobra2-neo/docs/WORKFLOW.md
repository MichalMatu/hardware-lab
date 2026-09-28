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
    -> M105 thermal-health gate
    -> XY home -> Z home -> pen up
    -> acknowledged artwork stream + periodic M105
    -> M400 -> final pen up
    -> structured result evidence
```

The normalized geometry layer represents drawable polylines/strokes independent of the Kobra. Text becomes vector strokes before machine fitting. Raster images require an explicit rendering strategy such as contours, hatching, crosshatching or stippling.

## Project-local prepare V1

`projects/kobra2-neo` owns the prepare-only `kobra-plot` CLI:

`SVG / text -> vpype source conversion -> normalized line SVG -> Kobra orientation/fit -> bounded G-code -> preview/report`

V1 uses pinned `vpype==1.15.0`. Raster and PDF extensions are detected but deliberately rejected until explicit line-rendering presets are implemented and tested.

Preparation never opens serial. Every prepare report keeps execution disabled; physical execution is a separate operator-approved command.

## Repository boundary

- `hardware-lab/projects/kobra2-neo` owns Kobra-specific conversion, calibration, limits, thermal-health policy, Marlin protocol policy and long-running plot streaming.
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
- homing is forbidden in prepare-generated artwork and belongs only to the approved live preamble.

The profile is stored at `config/kobra2_neo_pen.toml`. Unknown or missing keys fail closed.

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
8. Obtain explicit approval for the complete physical transaction.

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
3. queries `M105` and requires a plausible heater-off hotend temperature before any deliberate motion;
4. sends `G28 X Y` and waits for `ok`;
5. sends `G28 Z` and waits for `ok`;
6. selects absolute mode and raises the pen;
7. streams the already-approved artwork command by command, waiting for Marlin acknowledgement after every command;
8. periodically queries `M105` during long jobs so an E0 thermistor problem is surfaced before a later opaque firmware halt where possible;
9. treats firmware errors, resend requests and acknowledgement timeouts as terminal failures rather than blind retry opportunities;
10. treats `MINTEMP`, `MAXTEMP`, `Printer halted` and equivalent Marlin kill states as terminal, with final pen state unknown unless a later acknowledgement proves otherwise;
11. after the artwork `M400`, raises the pen again and waits for a final `M400`;
12. emits a terminal pen-up result only after that final safe state is acknowledged.

For a non-fatal streaming/transport failure after successful Z homing, the runner makes one bounded best-effort pen-up recovery attempt and reports whether it was acknowledged. After a Marlin kill state it does not pretend that additional motion is reliable.

## Structured progress and evidence

Long physical tasks must emit Local Agent native markers:

```text
[AGENT_PROGRESS] {"stage_name":"kobra-live",...}
```

The runner reports `PREFLIGHT_OK`, `PRINTER_IDENTIFIED`, `THERMAL_MONITOR_OK`, `HOMING_XY_OK`, `HOMING_Z_OK`, `PEN_UP_OK`, `DRAWING_STARTED`, periodic drawing/thermal progress and `COMPLETE_PEN_UP`. A fatal Marlin safety stop reports `FIRMWARE_HALTED_FINAL_PEN_STATE_UNKNOWN`.

Never infer a physical stage from process liveness or `seconds_since_output`. Say a stage passed only when its structured progress/result evidence exists.

## Fast-path transport rule

A new chat does not require a new host-ops probe when the host/cabling session is unchanged and the serial path is known. The live runner always performs its own `M115` identity check and `M105` thermal-health check before motion. Use host-ops discovery/probe only if the port is unknown, changed, ambiguous or the runner's identity check fails.

Never open a host-ops serial probe concurrently with an active live task on the same printer.

## Live validation milestones

- 2026-09-28: 10 cm `MongooseLemur.svg` outline completed all 7615 acknowledged commands in about 14 min 44 s and finished pen-up, validating the physical profile and acknowledgement-driven streaming model.
- 2026-09-28: the slow-start incident exposed missing durable execution/orchestration. `kobra-live`, structured progress and the golden runbook were added as corrective actions.
- 2026-09-28: `shaft-50x20-showcase` later reached command 3300/4345 before stock Marlin halted on `MINTEMP` for E0. That run finished failed after 801.199 s with no confirmed final pen-up. The thermal-health gate is therefore part of the permanent live contract; disabling thermal protection is not an acceptable workaround.

See `GOLDEN_LIVE_FLOW.md` for the authoritative operator flow and `INCIDENT_2026-09-28_SLOW_LIVE_START.md` for the incident review.