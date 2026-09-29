# Kobra 2 Neo handoff — 2026-09-29

## Start here

The machine is a dedicated pen plotter. A map plot completed successfully on 2026-09-29. The next attempted botanical plot did **not** reach physical execution; repeated failures happened in repository/task orchestration and artwork transfer, not in printer motion.

Do not resume the failed botanical base64/chunk workflow. Start the next artwork from a single normal source file and follow `WORKFLOW.md` exactly.

## Current physical contract

- Anycubic Kobra 2 Neo, stock Marlin `bugfix-2.1.x`.
- Original printhead/hotend removed.
- No hotend heater cartridge, hotend thermistor or printhead fans.
- Pen/marker is the active tool.
- Cylindrical magnetic/proximity sensor is verified as `z_min` and used for Z homing.
- Rear physical button maps to `z_max`.
- Idle hotend `T:0.00` is expected for the absent thermistor and is not itself a plotting blocker.
- Firmware-emitted `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or equivalent during an active transaction is still terminal.
- Before XY homing, clear the complete Y-bed path, especially the printer power cable.

## Current profile — source of truth

Read `config/kobra2_neo_pen.toml` before every implementation or documentation change. As of this handoff:

```text
hard envelope: X=3..223, Y=36..230 mm
normal drawing envelope: X=8..218, Y=41..225 mm
pen down: Z=2.97
pen up: Z=4.97   # exactly 2.00 mm lift
travel feed: 6000 mm/min
draw feed: 2400 mm/min
Z feed: 360 mm/min
orientation: flip Y; no XY swap; no X flip
end: M400
```

Changing the pen, holder, paper thickness/position or Z-reference geometry invalidates the Z calibration until rechecked.

## Known code gap on `main`

At the time of this handoff the committed profile already has the values above, but `main` still has implementation drift that must be resolved **before using unattended `kobra-live` for another physical job**:

- `src/kobra_live.py` still contains the older room-temperature `M105` gate and hard-coded `F180` pen-up feed in the currently visible `main` version.
- `src/kobra_plot.py` on `main` does not yet contain the tested contiguous-path merge helper.
- A Local Agent-only commit `9d4b995` (`Tune Kobra pen plotting motion`) passed the full local test suite (`33/33`) and contained those code/test corrections, but repeated non-fast-forward races prevented it from being merged to `main`.

Do not assume that local commit will still exist in a future session. Either recover/rebase it in one dedicated code-maintenance task or reimplement the small changes against current `main`, then run tests once and merge them **before** starting any artwork task.

## Canonical next-session sequence

### 0. MAINTENANCE GATE — only if code/profile disagree

This is a code-only step. No artwork transfer and no serial access.

- Confirm no Local Agent physical task is active.
- Compare `config/kobra2_neo_pen.toml`, `src/kobra_plot.py`, `src/kobra_live.py` and tests.
- Fix profile/runner/generator drift in one change.
- Run the test suite once.
- Merge/push the tested code.
- Stop if tests or push fail. Do not continue to artwork in the same failed task.

### 1. ARTWORK

- Produce/select the artwork.
- Convert it to one final pen-compatible SVG.
- Store/transfer it as **one normal file**.
- Record SHA-256.
- From this point the approved source is immutable.
- Never use manual gzip/base64 chunking or task-payload transport for artwork.

### 2. PREPARE — offline only

One task, one responsibility: source -> prepared job.

- No code edits.
- No commits/pushes required for the prepare operation itself.
- No serial port access.
- Normalize/vectorize as needed.
- Apply orientation/fit.
- Optimize/order paths; only merge path boundaries when endpoints truly touch within the tested tolerance.
- Generate G-code, preview and report.
- Validate command whitelist, Z values, feeds and XY bounds.
- Record source/profile/G-code hashes.
- Terminal state: `READY_TO_PRINT` or failure.

If PREPARE fails, stop. Diagnose that exact boundary before retrying. Do not create a chain of speculative task variants.

### 3. REVIEW / APPROVAL

Review the actual preview/report/G-code and confirm physical paper/tool state. Approval covers one complete transaction only.

### 4. PRINT

PRINT consumes an already prepared immutable job. It must never edit code, regenerate artwork, commit or push.

- Verify G-code/report/profile hashes.
- Ensure no competing serial task/session.
- Identify the physical printer with `M115`.
- Home XY, then Z.
- Raise to current pen-up Z using current profile Z feed.
- Stream the already-approved G-code with acknowledgement after each command.
- Treat firmware halt, resend/transport error or timeout as terminal.
- Finish pen-up + `M400` and require terminal evidence.

Physical status language is strict:

- say `drawing started` only after `DRAWING_STARTED` evidence;
- say `completed` only after `COMPLETE_PEN_UP` / equivalent final acknowledgement;
- process liveness is not physical progress.

## Fail-fast rule

There is no multi-hour recovery loop.

A stage failure stops the pipeline. One corrective retry is acceptable only after the exact root cause is identified and the fix is deterministic. If that retry fails, stop and hand off the blocker instead of creating more task variants.

Never combine code maintenance, repository synchronization, artwork transfer, preparation and physical printing in one task.

## What went wrong on 2026-09-29

The failed botanical attempt combined too many responsibilities and used an improvised asset-transfer mechanism. The observed failures were:

- stale test expectations after profile tuning;
- repeated `main` non-fast-forward races while a Local Agent task and GitHub API writes both changed the repository;
- truncated/invalid base64 artwork payload;
- missing chunk in a second chunked transfer attempt.

None of these required printer motion. The corrective design is the four-stage flow above with immutable single-file artwork and no repository writes inside PRINT.

## Handoff success criterion

The next session should not begin by trying to print. It should first read this file, `WORKFLOW.md`, and `config/kobra2_neo_pen.toml`, resolve the known code gap if it still exists, and only then accept a new artwork job.
