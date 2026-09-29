# Incident review — slow live plot start — 2026-09-28

> **Correction added 2026-09-29:** the current machine is a dedicated headless pen plotter. The original printhead is intentionally removed: no hotend heater, no hotend thermistor and no printhead fans. The active tool is a pen, and the cylindrical magnetic/proximity sensor is the verified `z_min`. Therefore idle `M105` hotend `T:0.00` is expected and must not block plotting. The historical `MINTEMP` event below proves only that an actual firmware kill during execution is terminal; it does not require reinstalling the thermistor. Current behavior is defined by `HARDWARE.md`, `SAFETY.md` and `GOLDEN_LIVE_FLOW.md`.

## Impact

An operator-approved Kobra 2 Neo drawing took more than three hours of conversation/workflow churn before the physical plot was finally started. The printer/calibration were not the main cause of the startup delay. The delay was orchestration and artifact-flow debt.

The eventual live run also exposed a separate firmware event: stock Marlin halted during the plot with `MINTEMP` on hotend sensor E0. The correct current interpretation is that a firmware kill during motion is terminal. It is not evidence that the intentionally absent hotend sensor must be restored for pen plotting.

## What happened

1. The originally referenced `shaft-120x20-technical-demo.job.json` was not a self-contained executable job. The expected complete G-code/generator artifact was unavailable; an earlier saved generator representation was truncated and could not reproduce the intended drawing exactly.
2. Instead of immediately converting that into one explicit blocker and one recovery path, the workflow spent too long regenerating/rechecking artifacts and revisiting already-known state.
3. Live execution had no committed project-local runner. Documentation explicitly described the successful serial streamer as temporary, so each live attempt required constructing an ad-hoc Local Agent command body.
4. One live attempt (`kobra2-neo-live-shaft50-showcase-20260928-20`) assumed the `hostops` executable was available in the `hardware-lab` worker PATH. That assumption was false and the task failed before any motion.
5. The correct worker boundary was then used: `host-ops` performed a generic serial/identity probe on its own binding, while the hardware-lab worker used the already-proven direct Marlin serial path.
6. Task `kobra2-neo-live-shaft50-showcase-20260928-21` then passed full G-code preflight, identified the physical printer with `M115`, completed XY homing, completed Z homing, raised the pen and started drawing.
7. The task streamed successfully through progress `3300/4345`. Marlin then returned `Error:MINTEMP triggered, system stopped! Heater_ID: E0` after command `G1 X151.15 Y83.71 F1200`.
8. The task terminated failed after `801.199 s`. Because Marlin had entered a halted state, there is no acknowledged final pen-up. The final physical pen state from that run is therefore **unknown**, not inferred.
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

## Separate runtime finding: E0 firmware halt

The prepared job contained no heater or extrusion commands and passed the command whitelist/bounds checks. During the live run, stock Marlin nevertheless emitted a `MINTEMP` kill for E0.

At the time of the incident, documentation incorrectly inferred that the next pen plot required a healthy connected hotend thermistor and stable room-temperature `M105` evidence. That inference is now superseded by the confirmed physical configuration:

- the original printhead/hotend is intentionally absent;
- there is no heater cartridge;
- there is no hotend thermistor;
- there are no printhead fans;
- the pen is the intended tool;
- idle `T:0.00` is expected.

The durable rule is narrower: if Marlin itself enters `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or another kill state **during an active command transaction**, stop and report final pen state unknown. Do not convert the expected absent thermistor into a pre-motion block.

The operator later reported that interacting with the printer-screen nozzle-temperature control preceded the thermal failure. Do not use nozzle-temperature controls or send heater commands in the pen-plotter configuration.

## Corrective actions completed

- Added committed project-local `kobra-live` runner in `hardware-lab`.
- Runner revalidates G-code/report before opening serial, can pin SHA-256, checks calibrated envelope/Z values and identifies the printer with `M115`.
- Runner handles the approved homing/start/stream/end sequence and waits for Marlin acknowledgement after each command.
- A firmware kill state is reported as `FIRMWARE_HALTED_FINAL_PEN_STATE_UNKNOWN`; it is not disguised as a successful recovery pen-up.
- Runner emits `[AGENT_PROGRESS]` checkpoints consumable by Local Agent heartbeat/status evidence.
- Added unit/preflight tests and CI compilation/validate-only coverage.
- Added `GOLDEN_LIVE_FLOW.md` with repository boundaries, fast path, evidence gates, fail-fast communication and recovery rules.
- Clarified in `host-ops` that `serial transact` is a bounded raw transaction/discovery primitive, not the Kobra long-running protocol executor.
- Added Local Agent physical-task progress guidance so physical stages are asserted only from structured evidence.
- Updated Kobra hardware and safety documentation with the confirmed headless pen-plotter configuration and corrected interpretation of `T:0.00`.

## Rules preventing recurrence

1. If a required artifact is missing/corrupt, say `BLOCKED` immediately with the missing artifact and the single recovery action. Do not spend hours silently reconstructing history.
2. If an approved job exists, do not write another ad-hoc streamer. Use `kobra-live`.
3. Do not assume a tool installed in one Local Agent binding is available in another. Keep cross-repo capability calls on their owning worker unless a committed dependency explicitly says otherwise.
4. Do not serial-probe a device concurrently with an active physical task.
5. Emit structured progress for long physical operations.
6. Never state `homing passed`, `drawing started` or `complete` from generic heartbeat/liveness alone.
7. Diagnose one failed boundary before retrying. No speculative retry chains.
8. Do not repeat host/device discovery solely because the chat changed. Re-prove only state that may actually have changed; `kobra-live` performs its own physical `M115` identity check before motion.
9. Do not block the current headless pen plotter on idle `M105 T:0.00`; the hotend thermistor is intentionally absent.
10. Do not send heater commands or use nozzle-temperature controls with the headless pen setup.
11. Treat an actual Marlin kill/halt during execution as terminal and do not issue speculative recovery motion after the firmware has halted.
12. Before XY homing, verify the whole bed path is clear, especially the rear Y path and printer power cable.

## Golden baseline

The current canonical hardware and live-flow contracts are `HARDWARE.md`, `SAFETY.md`, `WORKFLOW.md` and `GOLDEN_LIVE_FLOW.md`. This incident file remains historical evidence and must not override those current documents.
