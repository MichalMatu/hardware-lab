# Incident review — slow live plot start — 2026-09-28

## Impact

An operator-approved Kobra 2 Neo drawing took more than three hours of conversation/workflow churn before the physical plot was finally started. The printer/calibration were not the main cause of the startup delay. The delay was orchestration and artifact-flow debt.

The eventual live run also exposed a separate real hardware/firmware safety dependency: stock Marlin halted during the plot with `MINTEMP` on hotend sensor E0. That failure is independent of the earlier three-hour startup delay, but it must be part of the permanent live-flow contract.

## What happened

1. The originally referenced `shaft-120x20-technical-demo.job.json` was not a self-contained executable job. The expected complete G-code/generator artifact was unavailable; an earlier saved generator representation was truncated and could not reproduce the intended drawing exactly.
2. Instead of immediately converting that into one explicit blocker and one recovery path, the workflow spent too long regenerating/rechecking artifacts and revisiting already-known state.
3. Live execution had no committed project-local runner. Documentation explicitly described the successful serial streamer as temporary, so each live attempt required constructing an ad-hoc Local Agent command body.
4. One live attempt (`kobra2-neo-live-shaft50-showcase-20260928-20`) assumed the `hostops` executable was available in the `hardware-lab` worker PATH. That assumption was false and the task failed before any motion.
5. The correct worker boundary was then used: `host-ops` performed a generic serial/identity probe on its own binding, while the hardware-lab worker used the already-proven direct Marlin serial path.
6. Task `kobra2-neo-live-shaft50-showcase-20260928-21` then passed full G-code preflight, identified the physical printer with `M115`, completed XY homing, completed Z homing, raised the pen and started drawing.
7. The task streamed successfully through progress `3300/4345`. Marlin then returned `Error:MINTEMP triggered, system stopped! Heater_ID: E0` after command `G1 X151.15 Y83.71 F1200`.
8. The task terminated failed after `801.199 s`. Because Marlin had entered a halted safety state, there is no acknowledged final pen-up. The final physical pen state from that run is therefore **unknown**, not inferred.
9. Progress messages in the ad-hoc task were plain stdout. Local Agent already supports structured `[AGENT_PROGRESS]` markers, but they were not used. Remote heartbeat evidence therefore showed liveness/recent output but not the exact physical checkpoint.
10. An assistant status message consequently overstated evidence by saying homing had passed before the corresponding captured output was available. Later terminal result evidence did prove that homing had in fact succeeded, but the earlier claim was still unsupported at the time it was made.

## Root causes of the slow startup

### Primary

- No durable, committed live executor in the canonical Kobra project.
- No single documented fast path from approved immutable artifact to physical execution.
- Worker capability boundaries (`hardware-lab` vs `host-ops`) were known conceptually but not encoded strongly enough in the live runbook.
- The initial job artifact was not self-contained/reproducible.

### Contributing

- Repeated speculative recovery instead of fail-fast blocker reporting.
- Plain stdout instead of Local Agent structured progress markers.
- Evidence language was not strict enough: process liveness was treated as proof of a physical stage.
- A new chat/session was allowed to trigger redundant capability rediscovery instead of reusing known state and proving only the facts that could actually have changed.

## Separate runtime safety finding: E0 thermal monitor

The `MINTEMP` failure was not caused by heater commands in the artwork: the prepared job contained no heater or extrusion commands and passed the command whitelist/bounds checks. Stock Marlin nevertheless continues to enforce the hotend thermistor safety circuit during pen-only motion.

The repository does not yet prove whether the observed E0 `MINTEMP` came from an intermittent thermistor connector, sensor/wiring fault or another physical thermal-input issue. Do not guess and do not disable thermal protection to work around it.

Before the next physical plot, inspect the E0 thermistor wiring/connector and obtain stable room-temperature `M105` evidence. The durable live runner now makes that a pre-motion and periodic runtime gate.

## Corrective actions completed

- Added committed project-local `kobra-live` runner in `hardware-lab`.
- Runner revalidates G-code/report before opening serial, can pin SHA-256, checks calibrated envelope/Z values and identifies the printer with `M115`.
- Runner queries `M105` before motion and periodically during long streams; implausible temperature or Marlin thermal kill-state evidence terminates the job explicitly.
- Runner handles the approved homing/start/stream/end sequence and waits for Marlin acknowledgement after each command.
- A firmware kill state is reported as `FIRMWARE_HALTED_FINAL_PEN_STATE_UNKNOWN`; it is not disguised as a successful recovery pen-up.
- Runner emits `[AGENT_PROGRESS]` checkpoints consumable by Local Agent heartbeat/status evidence.
- Added unit/preflight tests and CI compilation/validate-only coverage.
- Added `GOLDEN_LIVE_FLOW.md` with repository boundaries, fast path, evidence gates, thermal-health gate, fail-fast communication and recovery rules.
- Clarified in `host-ops` that `serial transact` is a bounded raw transaction/discovery primitive, not the Kobra long-running protocol executor.
- Added Local Agent physical-task progress guidance so physical stages are asserted only from structured evidence.
- Updated Kobra hardware and safety documentation with the exact `MINTEMP` evidence and the unknown final pen state.

## Rules preventing recurrence

1. If a required artifact is missing/corrupt, say `BLOCKED` immediately with the missing artifact and the single recovery action. Do not spend hours silently reconstructing history.
2. If an approved job exists, do not write another ad-hoc streamer. Use `kobra-live`.
3. Do not assume a tool installed in one Local Agent binding is available in another. Keep cross-repo capability calls on their owning worker unless a committed dependency explicitly says otherwise.
4. Do not serial-probe a device concurrently with an active physical task.
5. Emit structured progress for long physical operations.
6. Never state `homing passed`, `drawing started` or `complete` from generic heartbeat/liveness alone.
7. Diagnose one failed boundary before retrying. No speculative retry chains.
8. Do not repeat host/device discovery solely because the chat changed. Re-prove only state that may actually have changed; `kobra-live` always performs its own physical `M115` identity check before motion.
9. A stable `M105` thermal-health check is mandatory before the next Kobra physical plot and is repeated during long plots.
10. Never disable Marlin thermal protection merely to keep a pen plot moving.

## Golden baseline

The post-incident baseline is the first `hardware-lab/main` revision after these corrective changes pass CI and the cross-repository documentation/branch audit is complete. That revision is the canonical Kobra workflow checkpoint for future sessions.

A golden software/workflow checkpoint does **not** mean the printer is currently cleared for another physical plot. After the 2026-09-28 `MINTEMP`, the next physical transaction remains blocked until the E0 thermal-monitor path is inspected and stable `M105` room-temperature evidence is obtained.