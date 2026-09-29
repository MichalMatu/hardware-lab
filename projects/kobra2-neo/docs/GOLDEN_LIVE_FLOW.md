# Golden live flow

Status: canonical PRINT-stage contract for the dedicated Kobra 2 Neo pen plotter.

This file starts only after ARTWORK, PREPARE and REVIEW / APPROVAL have completed. It does not describe source conversion, code maintenance or repository synchronization. Those belong to `WORKFLOW.md`.

## Preconditions

Before PRINT:

- `docs/HANDOFF.md` has been read for current implementation status;
- code, tests and `config/kobra2_neo_pen.toml` agree;
- a prepared immutable G-code/report pair exists;
- source/profile/G-code hashes are recorded;
- preview/report/G-code have been reviewed;
- the physical transaction has explicit operator approval;
- no competing Local Agent live task or serial session is active;
- the full XY travel path is clear, especially the rear Y-bed path and printer power cable.

If any precondition is false, PRINT does not start.

## Current hardware contract

The machine is a dedicated headless pen plotter:

- original printhead/hotend removed;
- no hotend heater cartridge;
- no hotend thermistor;
- no printhead fans;
- pen/marker active;
- cylindrical magnetic/proximity sensor is verified as `z_min` for Z homing;
- rear physical button maps to `z_max`.

Idle hotend `T:0.00` is expected because the thermistor is intentionally absent. That value alone must not block pen plotting. Heater commands remain forbidden. If Marlin actually emits `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or an equivalent halt during a live transaction, that transaction fails immediately.

## Current profile

`config/kobra2_neo_pen.toml` is the executable source of truth. At the 2026-09-29 handoff:

```text
hard envelope: X=3..223, Y=36..230 mm
normal drawing envelope: X=8..218, Y=41..225 mm
pen down: Z=2.97
pen up: Z=4.97
travel: 6000 mm/min
draw: 2400 mm/min
Z: 360 mm/min
orientation: flip Y; no XY swap; no X flip
end: M400
```

Do not copy older `Z6.12`, `F180`, `3000` or `1200` values from historical documents or old G-code into a new job.

## Immutable job gate

PRINT accepts only an already prepared job. It must not regenerate source geometry or modify project code.

Before opening serial, revalidate:

- report declares valid safety state;
- G-code hash matches the approved report/job identity;
- only approved artwork commands are present;
- no homing, heater, extrusion or relative-mode commands are embedded in artwork;
- XY remains inside the normal drawing envelope;
- Z and feed values match the current profile.

A future durable `plot-job.json` manifest may package these identities, but the invariant is already required: PRINT consumes one immutable prepared job.

## Serial / identity gate

- exactly one serial session;
- 115200 baud;
- port name is not identity;
- `M115` must identify the expected Anycubic Kobra before motion.

Do not open a second diagnostic serial session while PRINT is active.

## Physical execution sequence

The approved transaction is:

```text
M115 identity
G28 X Y
G28 Z
G90
pen up to current profile Z at current profile Z feed
stream approved artwork command-by-command, waiting for acknowledgement
artwork M400
final pen up
final M400 / terminal acknowledgement
```

Homing belongs to this live preamble, never to prepare-generated artwork.

## Progress evidence

Expected structured milestones are:

```text
PREFLIGHT_OK
PRINTER_IDENTIFIED
HOMING_XY_OK
HOMING_Z_OK
PEN_UP_OK
DRAWING_STARTED
DRAWING N/TOTAL
COMPLETE_PEN_UP
```

Communication must match evidence:

- never say "drawing" before `DRAWING_STARTED`;
- never say "completed" before `COMPLETE_PEN_UP` or equivalent final acknowledgement;
- daemon/process liveness is not proof of physical progress.

## Terminal failures

The current transaction stops on:

- printer identity mismatch;
- artifact/hash/profile mismatch;
- bounds or command-policy failure;
- serial acknowledgement timeout;
- resend/protocol error not explicitly supported by the runner;
- firmware `MINTEMP`, `MAXTEMP`, halt/kill;
- physical obstruction or other unsafe condition.

For a non-firmware transport failure after successful Z homing, one bounded best-effort pen-up recovery may be attempted if the runner can still communicate safely. After a firmware kill, do not claim recovery motion succeeded unless later acknowledgement proves it.

## Fail-fast orchestration

PRINT is never also a Git task.

During PRINT there are:

- no source transfers;
- no code edits;
- no tests;
- no commits;
- no pulls/rebases/pushes;
- no profile changes.

If PRINT fails, report the exact boundary and stop. One corrective retry may happen only after the root cause is fixed. A second failure ends the attempt and becomes a handoff item.

## Historical material

Dated incident/checkpoint documents remain useful evidence but are not live instructions. `docs/README.md` defines documentation authority. The current profile and this flow supersede old start sequences containing `Z6.12` or `F180`.