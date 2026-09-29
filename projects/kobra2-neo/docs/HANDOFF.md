# Kobra 2 Neo handoff — 2026-09-29

## Start here

The Kobra 2 Neo is a dedicated pen plotter. A map plot completed successfully on 2026-09-29. A later botanical attempt never reached physical execution; the failure was orchestration/artwork-transfer churn, not printer motion.

Do not resume the failed base64/chunk workflow. Use `WORKFLOW.md` exactly: one task, one responsibility.

## Current physical contract

- Anycubic Kobra 2 Neo, stock Marlin `bugfix-2.1.x`.
- Original printhead/hotend removed.
- No hotend heater cartridge, hotend thermistor or printhead fans.
- Pen/marker is the active tool.
- Cylindrical magnetic/proximity sensor is verified as `z_min` and used for Z homing.
- Rear physical button maps to `z_max`.
- Idle hotend `T:0.00` is expected and is not itself a plotting blocker.
- Firmware-emitted `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or equivalent during an active transaction is terminal.
- Before XY homing, clear the full Y-bed path, especially the printer power cable.

## Current profile — executable source of truth

Read `config/kobra2_neo_pen.toml` before changing code or documentation.

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

Changing pen, holder, paper thickness/position or Z-reference geometry invalidates dependent calibration until rechecked.

## Maintenance status

The previous profile/runner drift is closed on `main` by commit `7a3d510` (`Tune Kobra pen plotting motion`). The maintenance verification passed the full test suite (`33/33`), compileall, `kobra-plot doctor` and `git diff --check`, and the commit was pushed successfully.

That commit provides:

- headless `T:0.00` acceptance while retaining terminal handling for actual Marlin halt messages;
- profile-driven pen-up Z feed (`360` rather than hard-coded `180`);
- current profile values in tests;
- adjacent touching-path merge before preview/G-code generation.

A follow-up optimization that also reverses the next path when its **end** touches the previous path end was implemented locally as commit `b358070` and passed `35/35` tests. Two attempts to push that commit were rejected by GitHub with `Internal Server Error`. Per the fail-fast rule, no third push variant was attempted. This is a non-safety optimization blocker, not a blocker for the canonical prepare/live flow on `7a3d510`.

## Canonical flow

### 1. ARTWORK

- Produce/select artwork.
- Convert it to one final pen-compatible SVG.
- Transfer/store it as **one normal file**.
- Record SHA-256 and treat it as immutable.
- Never use manual gzip/base64 chunking or task-payload asset transport.

### 2. PREPARE — offline only

One task: source -> prepared job.

- No code edits.
- No commits/pushes.
- No serial access.
- Normalize/vectorize as needed.
- Apply orientation/fit and path optimization.
- Generate G-code, preview and report.
- Validate command whitelist, Z values, feeds and XY bounds.
- Record source/profile/G-code hashes.
- Terminal state: `READY_TO_PRINT` or failure.

If PREPARE fails, stop. One corrective retry is allowed only after the exact root cause is known and the fix is deterministic.

### 3. REVIEW / APPROVAL

Review the actual preview/report/G-code and confirm paper/tool state. Approval covers one complete immutable transaction.

### 4. PRINT

PRINT consumes the prepared immutable job and must not edit code, regenerate artwork, commit, pull/rebase or push.

- verify hashes and safety state;
- ensure no competing serial session;
- identify printer with `M115`;
- home XY then Z;
- raise to current pen-up using current Z feed;
- stream acknowledged immutable artwork;
- treat firmware halt/resend/transport timeout as terminal;
- finish pen-up + `M400` and require terminal evidence.

Say `drawing started` only after `DRAWING_STARTED`. Say `completed` only after `COMPLETE_PEN_UP` or equivalent final acknowledgement.

## New-flow smoke test

A read-only end-to-end smoke test on current `main` completed successfully on 2026-09-29:

```text
single SVG
  -> kobra-plot prepare
  -> preview/report/G-code
  -> profile/hash assertions
  -> kobra-live --validate-only
  -> READY_TO_PRINT
```

Measured prepare + live-preflight time: **6.971 s**. The generated job had safety `PASS`, bounded geometry and a pinned G-code SHA-256.

This proves that the deterministic offline path is comfortably inside the desired 30-second interaction budget before physical homing/stream start.

## Product UX target

The intended user experience is:

```text
upload image in ChatGPT
"drukuj"
  -> ARTWORK single-file SVG
  -> PREPARE + immutable hashes
  -> approval already implied by the explicit "drukuj" request for that image
  -> PRINT
  -> DRAWING_STARTED target: about 30 seconds from request under normal host/printer conditions
```

The remaining engineering work for that target is to make the ChatGPT-upload -> single SVG handoff deterministic and fast, then invoke PRINT as a separate bounded physical step. Do not weaken safety checks to hit the latency target.

## Fail-fast rule

There is no multi-hour recovery loop.

A stage failure stops that stage. One corrective retry is allowed only after exact diagnosis and a deterministic fix. If the retry fails, stop and record the blocker instead of creating more task variants.

## Next session

Read in this order:

1. `docs/HANDOFF.md`
2. `docs/WORKFLOW.md`
3. `config/kobra2_neo_pen.toml`
4. `docs/GOLDEN_LIVE_FLOW.md`

Do not start by inventing a new runner or transfer mechanism. Reuse the canonical stages above.