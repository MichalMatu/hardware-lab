# Chat -> print quickstart

This is the canonical fast path for the Kobra 2 Neo pen plotter.

## User-facing contract

Normal use is intentionally simple:

```text
1. User attaches one image in ChatGPT.
2. User writes: drukuj
3. The system prepares exactly that image and starts the printer.
```

Target under normal conditions: `DRAWING_STARTED` within about 30 seconds.

The explicit `drukuj` command is approval for one physical transaction derived from that exact attached image. Do not ask for a second confirmation unless the image, paper/tool state or safety contract is ambiguous.

## Internal stages

Keep these stages separate even though the UX is one command:

```text
ARTWORK -> PREPARE -> PRINT
```

### ARTWORK

- Use the exact attached image; do not substitute/regenerate another artwork.
- Convert/vectorize it to one final pen-compatible SVG.
- Transport it as one normal UTF-8 file only.
- Canonical inbox branch: `plot-inbox`.
- Preferred reusable path: `projects/kobra2-neo/inbox/current.svg`.
- Create/update that one file, record the resulting immutable commit SHA and source SHA-256.
- Never use gzip/base64 chunks, task JSON asset payloads or multi-file fragment transport.

### PREPARE

Run as a separate offline Local Agent task.

- Start from current `main`.
- Read the source from the exact pinned inbox commit, for example with `git show <commit>:projects/kobra2-neo/inbox/current.svg`.
- Verify source SHA-256 before conversion.
- Run `kobra-plot prepare` into a temporary job directory.
- Verify report safety, profile values, XY bounds and G-code SHA-256.
- Run `kobra-live --validate-only` using that exact G-code/report/profile.
- Required terminal evidence: `RESULT:READY_TO_PRINT`.
- No serial access, code edits, commits or pushes in PREPARE.

Current executable profile source of truth: `config/kobra2_neo_pen.toml`.

Expected calibrated values:

```text
pen down Z = 2.97
pen up Z = 4.97
travel = 6000 mm/min
draw = 2400 mm/min
Z feed = 360 mm/min
normal XY envelope = X 8..218, Y 41..225 mm
orientation = flip Y only
```

### PRINT

Run as a separate bounded physical Local Agent task consuming the already prepared immutable job.

- Do not regenerate artwork.
- Do not modify code or profile.
- Do not pull/rebase/push during the physical transaction.
- Verify the prepared G-code SHA-256 again.
- Enumerate candidate serial ports; never trust a remembered `/dev/cu.usbserial-*` path.
- Identify the Anycubic Kobra with `M115`; require exactly one matching printer.
- `M105` returning `T:0.00` is normal for this dedicated headless pen plotter.
- Actual firmware `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called`, resend or transport timeout is terminal.
- Home `G28 X Y`, then `G28 Z`.
- Raise to current pen-up Z using current Z feed.
- Stream only the immutable validated artwork and wait for Marlin acknowledgements.
- Finish with final pen-up and `M400`.

Evidence language is strict:

```text
PREFLIGHT_OK       -> offline/live validation passed
PRINTER_IDENTIFIED -> M115 matched
HOMING_XY_OK       -> XY homing acknowledged
HOMING_Z_OK        -> Z homing acknowledged
DRAWING_STARTED    -> first acknowledged G1 drawing move
COMPLETE_PEN_UP    -> successful physical completion
```

Never claim physical progress from daemon liveness alone.

## Proven reference run — 2026-09-29

The accepted botanical artwork from ChatGPT proved the complete path.

```text
source SHA-256: 8d3f26b2354b30a9cd0671e93c6c2137973ab23330cc0cecbd6cdb30ea28a945
inbox commit: de057fd3a4250dd795e421cdff1d9d087f954b5d
prepared geometry: 176 polylines / 2018 points
G-code commands: 2373
G-code SHA-256: 18989085c5031f3e55e60b9103450d18435a0f872451e48b737cdc453bb2a8f4
PREPARE + live validate-only: 2.780 s
```

`DRAWING_STARTED` marker handling was corrected on `main` by commit `ef7eea4` and the test suite passed.

The subsequent physical PRINT task `kobra2-neo-botanical-print-live-20260929-49` reached real streamed drawing progress, proving the physical leg as well.

## Fail-fast rule

No multi-hour recovery loops.

- First failure: stop that stage and identify the exact cause.
- One corrective retry is allowed only after a deterministic fix.
- If that retry also fails, stop and report the blocker.
- Never create chains of speculative task variants.

## New-chat behavior

When a new conversation starts with the starter prompt from `HANDOFF.md`, read this file first, inspect the attached image, and execute the fast path. Do not redesign the workflow unless current repository evidence proves the documented contract is broken.
