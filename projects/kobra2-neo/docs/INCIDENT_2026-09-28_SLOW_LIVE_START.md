# Incident review — slow live plot start — 2026-09-28

## Impact

An operator-approved Kobra 2 Neo drawing took more than three hours of conversation/workflow churn before the physical plot was finally started. The printer/calibration were not the main cause. The delay was orchestration and artifact-flow debt.

## What happened

1. The originally referenced `shaft-120x20-technical-demo.job.json` was not a self-contained executable job. The expected complete G-code/generator artifact was unavailable; an earlier saved generator representation was truncated and could not reproduce the intended drawing exactly.
2. Instead of immediately converting that into one explicit blocker and one recovery path, the workflow spent too long regenerating/rechecking artifacts and revisiting already-known state.
3. Live execution had no committed project-local runner. Documentation explicitly described the successful serial streamer as temporary, so each live attempt required constructing an ad-hoc Local Agent command body.
4. One live attempt (`kobra2-neo-live-shaft50-showcase-20260928-20`) assumed the `hostops` executable was available in the `hardware-lab` worker PATH. That assumption was false and the task failed before any motion.
5. The correct architecture was then used: `host-ops` performed a generic serial/identity probe on its own binding, while the hardware-lab worker used the already-proven direct Marlin serial path. Task `kobra2-neo-live-shaft50-showcase-20260928-21` started the approved physical transaction.
6. Progress messages in the ad-hoc task were plain stdout. Local Agent already supports structured `[AGENT_PROGRESS]` markers, but they were not used. Remote heartbeat evidence therefore showed liveness/recent output but not the exact physical checkpoint.
7. An assistant status message consequently overstated evidence by saying homing had passed when, at that moment, only liveness/recent-output evidence was directly visible.

## Root causes

### Primary

- No durable, committed live executor in the canonical Kobra project.
- No single documented fast path from approved immutable artifact to physical execution.
- Worker capability boundaries (`hardware-lab` vs `host-ops`) were known conceptually but not encoded strongly enough in the live runbook.

### Contributing

- Incomplete/non-self-contained original job artifact.
- Repeated speculative recovery instead of fail-fast blocker reporting.
- Plain stdout instead of Local Agent structured progress markers.
- Evidence language was not strict enough: process liveness was treated as proof of a physical stage.

## Corrective actions completed

- Added committed `kobra-live` runner in `hardware-lab`.
- Runner revalidates G-code/report before opening serial, can pin SHA-256, checks calibrated envelope/Z values, identifies the printer with `M115`, handles the approved homing/start/stream/end sequence and waits for Marlin acknowledgement after each command.
- Runner emits `[AGENT_PROGRESS]` checkpoints consumable by Local Agent heartbeat/status evidence.
- Added unit/preflight tests and CI compilation/validate-only coverage.
- Added `GOLDEN_LIVE_FLOW.md` with repository boundaries, fast path, evidence gates, fail-fast communication and recovery rules.
- Clarified that `host-ops serial transact` is a bounded raw transaction/discovery primitive, not the Kobra long-running protocol executor.
- Added Local Agent physical-task progress guidance so physical stages are asserted only from structured evidence.

## Rules preventing recurrence

1. If a required artifact is missing/corrupt, say `BLOCKED` immediately with the missing artifact and the single recovery action. Do not spend hours silently reconstructing history.
2. If an approved job exists, do not write another ad-hoc streamer. Use `kobra-live`.
3. Do not assume a tool installed in one Local Agent binding is available in another. Keep cross-repo capability calls on their owning worker unless a committed dependency explicitly says otherwise.
4. Do not serial-probe a device concurrently with an active physical task.
5. Emit structured progress for long physical operations.
6. Never state `homing passed`, `drawing started` or `complete` from generic heartbeat/liveness alone.
7. Diagnose one failed boundary before retrying. No speculative retry chains.

## Golden baseline

The post-incident baseline is the first `hardware-lab/main` revision after these corrective changes pass CI and the currently running live job has a terminal result recorded. That revision is the canonical Kobra workflow checkpoint for future sessions.
