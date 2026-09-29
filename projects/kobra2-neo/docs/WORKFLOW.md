# Canonical plotter workflow

This is the project architecture for every new Kobra 2 Neo pen job. It exists to keep artwork conversion, preparation and physical execution deterministic and observable.

## Core rule

**One task = one responsibility.**

Never combine code maintenance, artwork transfer, preparation, Git synchronization and physical printing in one task.

```text
MAINTENANCE (only if code/profile disagree)
        -> ARTWORK
        -> PREPARE
        -> REVIEW / APPROVAL
        -> PRINT
```

A failure stops the current stage. Do not silently continue into the next stage.

## 0. Maintenance

Use this only when code, tests, profile or documentation disagree.

- no artwork transfer;
- no serial access;
- no physical movement;
- no prepare/print work in the same task.

Resolve the mismatch against current `main`, run tests once and merge the tested change. If tests or repository synchronization fail, stop here.

The 2026-09-29 profile/runner drift was closed by `7a3d510`. `docs/HANDOFF.md` records current implementation status and any later non-blocking follow-up.

## 1. ARTWORK

The output of ARTWORK is one immutable source file plus identity metadata.

For ChatGPT-originated line art, the canonical transport proven on 2026-09-29 is:

```text
chat image
  -> one final pen-compatible SVG
  -> one normal file on branch plot-inbox
  -> immutable commit + path
  -> SHA-256 verification on Local Agent host
```

Rules:

- use one normal file, preferably SVG;
- finish vectorization/simplification before declaring the source approved;
- record SHA-256;
- pin the exact commit/path used by PREPARE;
- do not mutate the approved source afterward;
- never transport artwork with manually split gzip/base64 files, task JSON blobs or chains of partial repository writes;
- `samples/` is for durable examples, not transport staging;
- `inbox/` / `plot-inbox` is the transient single-file artwork handoff path.

Before PREPARE, reconstruct/fetch the source from the pinned commit and verify its SHA-256. A mismatch stops the stage.

### Proven ingestion evidence

The accepted botanical image from ChatGPT was converted to `botanical-chat-ingest.svg` and stored as one file at commit `de057fd3a4250dd795e421cdff1d9d087f954b5d`.

```text
source SHA-256: 8d3f26b2354b30a9cd0671e93c6c2137973ab23330cc0cecbd6cdb30ea28a945
Git blob SHA-1: 772d7189578ed68a79b135179900b64470a1f6e1
```

Local and GitHub Git-blob identities matched exactly, and Local Agent independently verified the SHA-256 before preparation.

## 2. PREPARE

PREPARE is offline only. It transforms one verified source into one prepared immutable job.

It may:

- normalize/vectorize supported input;
- simplify/order geometry;
- apply current orientation and machine fit;
- conservatively merge touching path boundaries;
- generate preview, report and G-code;
- compute hashes and geometry statistics;
- validate bounds, feeds, Z values and command vocabulary.

It must not:

- edit project code;
- commit/push as part of normal preparation;
- open a serial port;
- home or move the printer;
- change profile values;
- decide that a different source artifact should be used.

Expected output:

```text
source.svg
normalized.svg
preview.svg
output.gcode
report.json
```

The terminal state is `READY_TO_PRINT` or failure.

The 2026-09-29 chat-ingestion smoke test produced:

```text
176 polylines
2018 points
2373 G-code commands
bounds X=22.57..203.43, Y=41.00..225.00 mm
safety PASS
G-code SHA-256 18989085c5031f3e55e60b9103450d18435a0f872451e48b737cdc453bb2a8f4
PREPARE + kobra-live --validate-only: 2.780 s
RESULT: READY_TO_PRINT
```

This test opened no serial port and caused no printer motion.

## 3. Review / approval

Review the actual prepared artifacts, not a verbal description of what should exist.

Verify:

- source hash and intended artwork;
- preview orientation and scale;
- report bounds and geometry statistics;
- current profile identity;
- complete G-code policy;
- no homing/heater/extrusion commands inside artwork;
- calibrated Z and feeds;
- physical paper/tool state;
- clear XY travel path, especially rear Y-bed travel and the power cable.

An explicit user command such as `drukuj` for the selected image may authorize one complete physical transaction for exactly the immutable job derived from that image, provided the preparation/revalidation gates above pass.

## 4. PRINT

PRINT consumes an already prepared and approved job. It is not a generation or repository-management stage.

PRINT must not:

- modify code;
- regenerate/re-vectorize artwork;
- commit/push/pull/rebase;
- change profile values;
- choose a different source artifact.

Before motion:

1. verify no competing serial/live task exists;
2. verify source/G-code/report/profile identities and safety state;
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

`config/kobra2_neo_pen.toml` is executable truth:

```text
hard envelope: X=3..223, Y=36..230 mm
normal drawing envelope: X=8..218, Y=41..225 mm
travel feed: 6000 mm/min
draw feed: 2400 mm/min
Z feed: 360 mm/min
pen up: Z=4.97
pen down: Z=2.97
orientation: flip Y; no XY swap; no X flip
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

Idle hotend `T:0.00` is expected and is not itself a pre-motion fault. A firmware halt actually emitted during a transaction remains terminal. Heater commands are forbidden.

## Evidence language

Physical claims require explicit evidence:

- `PREFLIGHT_OK` = preflight only;
- `PRINTER_IDENTIFIED` = `M115` matched;
- `HOMING_XY_OK` / `HOMING_Z_OK` = acknowledged homing;
- `DRAWING_STARTED` = first drawing stage evidence;
- `COMPLETE_PEN_UP` = terminal successful completion.

Process liveness, daemon state or elapsed time is not physical progress.

Before using `DRAWING_STARTED` for latency benchmarking, ensure the current runner emits it on the first acknowledged drawing move (`G1`), not on an arbitrary command index. `docs/HANDOFF.md` tracks the current status of that small follow-up.

## 30-second product target

The intended UX is:

```text
attach image in ChatGPT
"drukuj"
    -> ARTWORK single SVG
    -> immutable one-file transfer
    -> PREPARE / hashes / preflight
    -> PRINT
    -> DRAWING_STARTED ≈ 30 s target
```

The chat-artwork-to-`READY_TO_PRINT` boundary is now proven. The remaining benchmark is physical PRINT startup latency; do not weaken any safety/identity gate to meet the target.

## Fail-fast rule

There is no multi-hour speculative recovery loop.

- First failure: stop the stage and identify the exact root cause.
- One corrective retry is allowed only after a deterministic fix to that root cause.
- If that retry fails, stop and hand off the blocker.
- Do not create a sequence of task variants hoping one works.
- Never claim a physical stage without corresponding evidence.

## Repository ownership

- `hardware-lab/projects/kobra2-neo` owns Kobra-specific conversion, profile, calibration, G-code policy and live protocol.
- `host-ops` owns generic host/device capabilities only.
- Local Agent owns scheduling/watchdogs/evidence, not ad-hoc Kobra business logic.

See `GOLDEN_LIVE_FLOW.md` for the live operator contract and `HANDOFF.md` for current implementation status.