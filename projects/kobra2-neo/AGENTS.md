# Kobra 2 Neo project rules

These rules apply to everything under `projects/kobra2-neo/`.

## Start / authority

Before continuing Kobra work, read:

1. `docs/HANDOFF.md`
2. `docs/WORKFLOW.md`
3. `config/kobra2_neo_pen.toml`
4. `docs/GOLDEN_LIVE_FLOW.md`

`config/kobra2_neo_pen.toml` is the executable source of truth for current plotting values. If profile, code, tests or docs disagree, stop before physical execution and resolve the mismatch in a dedicated maintenance change.

Historical dated incident/checkpoint files are evidence only, not operator instructions.

## Core workflow rule

**One task = one responsibility.**

Use these explicit stages:

```text
MAINTENANCE (only if needed) -> ARTWORK -> PREPARE -> REVIEW/APPROVAL -> PRINT
```

Never combine code edits, asset transfer, preparation, Git synchronization and physical printing in one task.

A stage failure stops the pipeline. One corrective retry is allowed only after the exact root cause is identified and the fix is deterministic. If the retry fails, stop and hand off the blocker. Do not create speculative retry chains.

## Repository boundary

- Keep Kobra-specific hardware state, calibration, CAD, G-code policy, Marlin protocol policy and tool design in this project.
- Treat `host-ops` as an external generic machine/device capability layer; do not put Kobra-specific plotting policy into `host-ops` core.
- Local Agent owns task scheduling/watchdogs/evidence, not Kobra protocol logic.
- Never assume a command available in one worker binding is available in another.

## Artwork transport

- The output of ARTWORK is one immutable normal source file plus SHA-256.
- Prefer one pen-compatible SVG.
- Do not transport generated artwork using hand-built gzip/base64 chunks, task JSON blobs or sequences of partial GitHub files.
- If a one-file transfer path is unavailable, stop before PREPARE and choose a proper transfer mechanism.
- `samples/` is for durable examples, not temporary chunk staging.

## PREPARE

`kobra-plot` is offline and serial-free.

PREPARE may normalize/vectorize, simplify/order paths, fit/orient geometry, generate preview/report/G-code and validate safety. It must not edit code, commit/push as part of normal preparation, access serial or approve physical execution.

Prepared artwork contains no homing. It must fail closed on heaters, extrusion, unexpected relative mode, out-of-envelope motion, unexpected Z values or unexpected feeds.

Path-lift optimization may merge consecutive path boundaries only when endpoints genuinely touch within a tested conservative tolerance; never draw an unintended connector across a real gap.

## Current physical profile

At the 2026-09-29 handoff the committed profile is:

```text
hard envelope: X=3..223, Y=36..230 mm
normal drawing envelope: X=8..218, Y=41..225 mm
pen down: Z=2.97
pen up: Z=4.97
travel feed: 6000 mm/min
draw feed: 2400 mm/min
Z feed: 360 mm/min
orientation: flip Y; no XY swap; no X flip
end: M400
```

Do not copy old `Z6.12`, `F180`, `3000` or `1200` values from historical logs into new jobs.

Changing pen, holder, paper thickness/placement, tool mechanics or Z-reference geometry invalidates dependent calibration until revalidated.

## Current hardware contract

- Dedicated pen plotter, stock Marlin baseline.
- Original printhead/hotend removed.
- No hotend heater cartridge, hotend thermistor or printhead fans.
- Pen/marker is the active tool.
- Cylindrical magnetic/proximity sensor is verified as `z_min` for Z homing.
- Rear physical button maps to `z_max`.
- Idle hotend `T:0.00` is expected for the absent thermistor and must not by itself block plotting.
- Heater commands remain forbidden.
- Firmware-emitted `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or equivalent during an active transaction is terminal.

Before `G28 X Y`, verify the complete travel path is physically clear, especially the rear Y-bed path and printer power cable.

## PRINT / live hardware safety

PRINT consumes one already prepared, reviewed and approved immutable job.

PRINT must not modify code, regenerate artwork, change profile values, commit, pull/rebase or push.

The approved transaction is bounded to:

```text
M115 identity -> G28 X Y -> G28 Z -> pen up -> immutable artwork stream -> M400 -> final pen up -> terminal acknowledgement
```

Additional rules:

- explicit operator approval is required before the full physical transaction;
- an explicit `drukuj` request for one specific uploaded image may serve as that approval only after the immutable prepared job and automatic safety checks exist for that exact image;
- never identify the printer solely by CH340 VID/PID or remembered port path; require `M115` identity;
- never open a second serial session while a live task owns the printer;
- wait for Marlin acknowledgement according to the live runner contract;
- use `M400` where planner completion must be proven;
- a firmware halt, timeout or unsupported protocol/resend condition terminates the transaction.

## Live orchestration and evidence

- Read daemon/run state before enqueueing a physical task; an active physical task wins.
- Long physical operations must emit structured `[AGENT_PROGRESS]` evidence.
- Do not claim `homing passed`, `drawing started` or `complete` from process liveness, heartbeat age or recent stdout.
- Say `drawing started` only after `DRAWING_STARTED` evidence.
- Say `complete` only after `COMPLETE_PEN_UP` or equivalent final acknowledgement.
- Report missing artifacts, repository mismatch, worker mismatch, serial identity failure, firmware errors and timeouts immediately.

## Current implementation status

- `main` commit `7a3d510` aligns headless `T:0.00` handling and profile-driven Z feed with the current profile and passed the full maintenance suite (`33/33`, compileall, doctor, diff-check).
- A read-only `SVG -> PREPARE -> kobra-live --validate-only -> READY_TO_PRINT` smoke test completed in 6.971 s.
- Optional end-touch path reversal was implemented/tested locally as `b358070` (`35/35`) but two GitHub pushes returned `Internal Server Error`; per fail-fast rules that push was not retried again. Do not silently recreate a retry loop around it.

## Fast-path product target

The intended interaction is: user uploads an image in ChatGPT and says `drukuj`; normal conditions should reach `DRAWING_STARTED` in about 30 seconds.

Achieve this by making ARTWORK transfer deterministic and fast, not by weakening PREPARE or PRINT safety. The one-command UX may orchestrate several internal stages, but the responsibility boundaries remain separate and observable.

## Mechanical design

- Preserve editable Fusion 360/CAD source as the canonical mechanical artifact.
- Keep moving mass low and preserve cable clearance.
- Document any change affecting pen-tip envelope or Z calibration.
- Do not discard the working pen baseline before a replacement tool is physically validated.

## Software

- Stock Marlin remains the firmware baseline until a concrete requirement justifies change.
- `kobra-plot` prepares jobs; `kobra-live` executes only already-prepared and approved jobs.
- Safety/calibration state must be explicit rather than inferred from printer-model defaults.