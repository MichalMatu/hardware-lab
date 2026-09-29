# Canonical plotter workflow

This is the project architecture for every new Kobra 2 Neo pen job. The purpose is to make the path deterministic, observable and resistant to the orchestration failures seen on 2026-09-29.

## Core rule

**One task = one responsibility.**

Never combine code maintenance, artwork transfer, preparation, Git synchronization and physical printing in one task.

The canonical flow is:

```text
MAINTENANCE GATE (only when code/profile disagree)
        -> ARTWORK
        -> PREPARE
        -> REVIEW / APPROVAL
        -> PRINT
```

A failure stops the current stage. Do not silently continue into the next stage.

## 0. Maintenance gate

Use this only when code, tests, profile or documentation disagree.

This is a code-only stage:

- no artwork transfer;
- no serial access;
- no physical movement;
- no attempt to prepare or print a job.

Resolve the mismatch against current `main`, run the test suite once, and merge the tested change. If tests or repository synchronization fail, stop here.

As of the 2026-09-29 handoff, `docs/HANDOFF.md` records a known implementation gap that must be resolved before the next unattended `kobra-live` job.

## 1. ARTWORK

The output of ARTWORK is one immutable source file and its SHA-256.

Rules:

- use one normal file, preferably a pen-compatible SVG;
- finish vectorization/simplification before declaring the source approved;
- record SHA-256;
- do not mutate the approved source afterward;
- do not transport artwork with manually split gzip/base64 files, task JSON blobs or a chain of partial repository writes;
- if a one-file transfer path is unavailable, stop and choose a proper transfer method before PREPARE.

`projects/kobra2-neo/samples/` is for durable examples, not a temporary chunk-transfer protocol.

## 2. PREPARE

PREPARE is offline only. It transforms one source file into one prepared immutable job.

It may:

- normalize/vectorize supported input;
- simplify/order geometry;
- apply current orientation and machine fit;
- merge consecutive path boundaries only when endpoints genuinely touch within a tested tolerance;
- generate preview, report and G-code;
- compute hashes and geometry statistics;
- validate bounds, feeds, Z values and command vocabulary.

It must not:

- edit project code;
- commit or push as part of normal preparation;
- open a serial port;
- home or move the printer;
- decide that the physical job is approved.

Expected output is equivalent to:

```text
source.svg
normalized.svg
preview.svg
output.gcode
report.json
```

The terminal state is `READY_TO_PRINT` or failure. If PREPARE fails, stop and diagnose that exact boundary.

## 3. Review / approval

Review the actual prepared artifacts, not a verbal description of what should exist.

Before approval verify:

- source hash and intended artwork;
- preview orientation and scale;
- report bounds and geometry statistics;
- current profile identity;
- complete G-code command policy;
- no homing/heater/extrusion commands inside artwork;
- calibrated Z and feed values;
- physical paper/tool state;
- clear XY travel path, especially the rear Y-bed path and power cable.

Approval authorizes one complete physical transaction for that immutable prepared job.

## 4. PRINT

PRINT consumes an already prepared and approved job. It is not a generation or repository-management stage.

PRINT must not:

- modify code;
- regenerate or re-vectorize artwork;
- commit or push;
- change profile values;
- select a different source artifact.

Before motion:

1. verify no competing serial/live task exists;
2. verify G-code/report/profile hashes and safety state;
3. open exactly one serial session at 115200;
4. prove physical identity with `M115`;
5. ensure the current headless hardware contract applies.

Then execute:

```text
G28 X Y
G28 Z
pen up using current profile
stream immutable artwork with acknowledgement after every command
M400
final pen up
final M400 / terminal acknowledgement
```

A firmware-emitted `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called`, resend/transport error or acknowledgement timeout is terminal for that transaction.

## Current profile contract

The executable source of truth is `config/kobra2_neo_pen.toml`. At the 2026-09-29 handoff it contains:

```text
hard envelope: X=3..223, Y=36..230 mm
normal drawing envelope: X=8..218, Y=41..225 mm
travel feed: 6000 mm/min
draw feed: 2400 mm/min
Z feed: 360 mm/min
pen up: Z=4.97
pen down: Z=2.97
orientation: flip Y, no XY swap, no X flip
end: M400
```

Documentation and implementation must match this profile before PRINT.

## Hardware mode

The machine is a dedicated pen plotter:

- original printhead/hotend removed;
- no hotend heater;
- no hotend thermistor;
- no printhead fans;
- pen/marker active;
- cylindrical magnetic/proximity sensor is verified `z_min` for Z homing.

Idle hotend `T:0.00` is expected for the absent thermistor and is not itself a pre-motion fault. A firmware halt actually emitted during a transaction remains terminal. Heater commands are forbidden.

## Prepared G-code policy

Prepared artwork may use only the project-approved motion vocabulary. Homing belongs to the PRINT preamble, never inside artwork. Heater/extrusion commands and relative-mode surprises fail closed. Every XY move must remain within the current normal drawing envelope; Z and feeds must match the active profile.

## Evidence language

Physical claims require explicit evidence:

- `PREFLIGHT_OK` means only offline/live preflight passed;
- `PRINTER_IDENTIFIED` means `M115` matched;
- `HOMING_XY_OK` / `HOMING_Z_OK` mean those homing commands were acknowledged;
- `DRAWING_STARTED` is required before saying the plot is drawing;
- `COMPLETE_PEN_UP` or equivalent final acknowledgement is required before saying the plot completed.

Process liveness, daemon state or elapsed time is not physical progress.

## Fail-fast rule

There is no multi-hour speculative recovery loop.

- First failure: stop the stage and identify the exact root cause.
- One corrective retry is allowed only after a deterministic fix to that root cause.
- If the corrective retry fails, stop and hand off the blocker.
- Do not create a sequence of variant tasks hoping one will work.
- Never claim a physical stage that has no structured evidence.

## Repository ownership

- `hardware-lab/projects/kobra2-neo` owns Kobra-specific conversion, profile, calibration, G-code policy and live protocol.
- `host-ops` owns generic host/device capabilities only.
- Local Agent owns scheduling/watchdogs/evidence, not ad-hoc Kobra business logic.

See `GOLDEN_LIVE_FLOW.md` for the exact live operator contract and `HANDOFF.md` for current implementation status.