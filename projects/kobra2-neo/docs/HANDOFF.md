# Kobra 2 Neo handoff — 2026-09-29

## Start here

The Kobra 2 Neo is a dedicated pen plotter. The canonical architecture is now proven through `READY_TO_PRINT` for an artwork originating in this ChatGPT conversation.

Do not resume the failed base64/chunk workflow. Use one normal immutable source file and the staged flow in `WORKFLOW.md`.

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

The previous profile/runner drift was closed on `main` by commit `7a3d510` (`Tune Kobra pen plotting motion`). Verification passed `33/33` tests, compileall, `kobra-plot doctor` and `git diff --check`.

That baseline provides:

- headless `T:0.00` acceptance while actual Marlin halt messages remain terminal;
- profile-driven pen-up Z feed (`360`);
- current profile values in tests;
- conservative adjacent touching-path merge before preview/G-code generation.

A non-safety path-ordering enhancement (`b358070`) passed `35/35` locally but two Git pushes were rejected with GitHub `Internal Server Error`; it is not required for the canonical flow.

A small follow-up task to make `DRAWING_STARTED` correspond exactly to the first acknowledged `G1` has been queued on `agent-control` as `kobra2-neo-drawing-start-marker-20260929-48`. At this handoff it had not yet been picked up by the daemon. Do not duplicate it. Resolve/check that one task before using `DRAWING_STARTED` for latency benchmarking.

## Proven ChatGPT artwork ingestion — 2026-09-29

The exact accepted botanical artwork from this conversation was used to prove the missing asset path.

Chat-side vector artifact:

```text
local source: botanical-chat-ingest.svg
source SHA-256: 8d3f26b2354b30a9cd0671e93c6c2137973ab23330cc0cecbd6cdb30ea28a945
Git blob SHA-1: 772d7189578ed68a79b135179900b64470a1f6e1
```

It was transferred as **one normal UTF-8 SVG**, not chunks:

```text
branch: plot-inbox
commit: de057fd3a4250dd795e421cdff1d9d087f954b5d
path: projects/kobra2-neo/inbox/botanical-chat-ingest.svg
```

GitHub returned the same Git blob SHA as the local file, proving exact byte identity after transport.

Local Agent task `kobra2-neo-chat-to-ready-smoke-20260929-47` then fetched that exact immutable commit, reconstructed the source with `git show`, verified the SHA-256, and ran the normal offline pipeline.

Result:

```text
source SHA-256: 8d3f26b2354b30a9cd0671e93c6c2137973ab23330cc0cecbd6cdb30ea28a945
geometry: 176 polylines, 2018 points
draw distance: 9508.629 mm
travel distance: 1676.709 mm
bounds: X=22.57..203.43, Y=41.00..225.00 mm
G-code commands: 2373
G-code SHA-256: 18989085c5031f3e55e60b9103450d18435a0f872451e48b737cdc453bb2a8f4
safety: PASS
kobra-live preflight: PASS
terminal result: READY_TO_PRINT
PREPARE + live validate-only: 2.780 s
whole Local Agent command: 3.960 s
```

No serial port was opened and no printer motion occurred during this smoke test.

This proves the full boundary:

```text
image available in ChatGPT
  -> one pen-compatible SVG
  -> one GitHub file on plot-inbox
  -> immutable commit + source SHA
  -> Local Agent fetches exact commit
  -> source hash verification
  -> kobra-plot PREPARE
  -> preview/report/G-code
  -> kobra-live --validate-only
  -> READY_TO_PRINT
```

## Canonical user-facing target

The desired interaction is:

```text
upload image in ChatGPT
"drukuj"
  -> create one SVG
  -> transfer one immutable file
  -> PREPARE + hashes + preflight
  -> PRINT
  -> DRAWING_STARTED target: about 30 seconds from request under normal host/printer conditions
```

The offline portion is now comfortably inside that budget. The next benchmark is the physical PRINT portion from an already validated job to the first acknowledged drawing move.

For line-art images, use the proven single-SVG path. Raster/photo rendering remains a separate renderer-quality problem and must not be hidden inside PRINT.

## Canonical stages

### 1. ARTWORK

- Produce/select artwork.
- Convert it to one final pen-compatible SVG.
- Store it as one normal file on `plot-inbox` or another explicit one-file transport.
- Record SHA-256 and immutable commit/path identity.
- Never use manual gzip/base64 chunking or task-payload asset transport.

### 2. PREPARE — offline only

One task: source -> prepared job.

- no code edits;
- no repository mutation;
- no serial access;
- verify source hash before conversion;
- normalize/vectorize, fit and optimize;
- generate G-code, preview and report;
- validate command whitelist, Z values, feeds and XY bounds;
- pin the G-code hash;
- terminal state: `READY_TO_PRINT` or failure.

### 3. REVIEW / APPROVAL

Review the actual prepared artifacts and confirm paper/tool state. An explicit `drukuj` request for the selected image can serve as approval for that single immutable transaction once the system has prepared and revalidated exactly that image/job.

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

## Fail-fast rule

There is no multi-hour recovery loop.

A stage failure stops that stage. One corrective retry is allowed only after exact diagnosis and a deterministic fix. If the retry fails, stop and record the blocker instead of creating more task variants.

## Next session / next physical test

Read in this order:

1. `docs/HANDOFF.md`
2. `docs/WORKFLOW.md`
3. `config/kobra2_neo_pen.toml`
4. `docs/GOLDEN_LIVE_FLOW.md`

Before measuring `drukuj -> DRAWING_STARTED`, first check the existing marker-fix task rather than enqueueing a duplicate. Then use the proven single-file artwork path above and keep PRINT as its own bounded physical task.