# Golden checkpoint — Kobra 2 Neo — 2026-09-28

This file names the post-incident software/workflow baseline for the Kobra 2 Neo pen plotter. The commit that adds this file is the canonical `golden` checkpoint for future sessions.

It is a **software/workflow checkpoint**, not a statement that the physical printer is currently cleared for another plot.

## Cross-repository baseline

- `hardware-lab`: baseline immediately before this checkpoint file: `c5606d244816cf297f7a854c7ea31d31dae5e270` (`Align workflow with thermal live gate`). The commit containing this file supersedes that SHA as the named hardware-lab checkpoint.
- `host-ops`: `20a27f7b930cf083c7ea998508f8b4b2f1153ea6` (`Clarify serial boundary for downstream live protocols`).
- `local-agent`: `72f0a813aafc86ff2fe77fe8e3fe05dd33e008ad` (`Document observable physical task progress`).

## Golden Kobra execution contract

The canonical approved-job path is now:

```text
prepared immutable G-code + report
    -> kobra-live offline revalidation
    -> SHA-256 pin when available
    -> explicit serial device
    -> M115 physical identity
    -> M105 heater-off thermal-health gate
    -> G28 X Y
    -> G28 Z
    -> pen up
    -> acknowledged artwork stream
    -> periodic M105 + structured progress
    -> artwork M400
    -> final pen up
    -> final M400
    -> terminal COMPLETE_PEN_UP evidence
```

`kobra-live` is committed in `projects/kobra2-neo/src/kobra_live.py`. Rebuilding a long ad-hoc Python serial streamer inside a Local Agent task is no longer the normal flow.

## Evidence contract

A physical stage is reported only from explicit structured evidence or the terminal result. Generic `running`, PID, heartbeat age or `seconds_since_output` proves liveness only.

Long Kobra jobs emit Local Agent native `[AGENT_PROGRESS]` markers including:

- `PREFLIGHT_OK`
- `PRINTER_IDENTIFIED`
- `THERMAL_MONITOR_OK`
- `HOMING_XY_OK`
- `HOMING_Z_OK`
- `PEN_UP_OK`
- `DRAWING_STARTED`
- `DRAWING N/TOTAL`
- `COMPLETE_PEN_UP`

Marlin thermal/kill states are terminal and explicitly report final pen state unknown.

## Repository ownership

- `hardware-lab/projects/kobra2-neo` owns Kobra calibration, bounds, G-code validation, thermal-health policy and the long-running Marlin protocol.
- `host-ops` owns generic host/device discovery and bounded raw serial transactions. It is not the Kobra session runner.
- `local-agent` owns binding/scheduling/watchdogs and durable run/result evidence.
- Do not assume executables installed in one repository worker are present in another worker PATH.

## Branch hygiene at audit time

- `hardware-lab`: only `main` and required `agent-control`.
- `host-ops`: only `main` and required `agent-control`.
- `local-agent`: infrastructure/state branches `chat-bridge-state` and `operator-control` remain; diverged development branches with unique commits remain; `fix/chat-bridge-grouped-turn-fallback` and `fix/chat-bridge-mixed-dom-ordering` were identified as fully contained in `main` and therefore stale. The connected GitHub control surface used for this audit did not expose branch/ref deletion, so those two refs were not falsely claimed as deleted.

## CI / validation status at checkpoint creation

- `hardware-lab` Kobra offline gate was green on `c5606d244816cf297f7a854c7ea31d31dae5e270`, including compile, tests and committed `kobra-live --validate-only` coverage. Gitleaks was also green.
- `local-agent` CI was green on `72f0a813aafc86ff2fe77fe8e3fe05dd33e008ad`.
- `host-ops` global `quality` was red on `20a27f7b930cf083c7ea998508f8b4b2f1153ea6`, but the immediately preceding main revision was already red as well; the documentation-only serial-boundary change did not introduce the first failure. Exact root cause remained a separate repository CI debt because GitHub job logs were unavailable through the connected control surface and the host-ops worker was already occupied by another existing host task. A no-write local quality audit was queued behind that task rather than interrupting it.

Therefore this checkpoint means **the Kobra flow and its direct Local Agent evidence contract are golden**, not that every unrelated check in every touched repository is globally green.

## Incident result that this checkpoint preserves

Live task `kobra2-neo-live-shaft50-showcase-20260928-21`:

- passed G-code preflight;
- identified Anycubic Kobra via `M115`;
- completed XY and Z homing;
- raised the pen;
- started drawing;
- reached progress `3300/4345`;
- then Marlin emitted `Error:MINTEMP triggered, system stopped! Heater_ID: E0`;
- task failed after `801.199 s`;
- final pen-up was **not** acknowledged, so final physical pen state is unknown.

This runtime fault is separate from the earlier multi-hour startup-flow failure.

## Physical clearance

Do not start another physical plot from this checkpoint until the E0 hotend thermistor/sensor wiring/connector is inspected and stable room-temperature `M105` readings are obtained.

Do not disable Marlin thermal protection as a workaround.

## Rule for future sessions

For an already prepared and approved job, the expected path is one project-local live task using `kobra-live`. If a prerequisite fails, report the exact blocker immediately and stop at that boundary. Do not spend hours silently regenerating state, bouncing between repository workers or creating speculative serial-task variants.
