# Golden checkpoint — Kobra 2 Neo — 2026-09-28

> **Hardware-contract correction — 2026-09-29:** this checkpoint is historical evidence of the 2026-09-28 software/workflow state. Its original requirement for a healthy E0 hotend thermistor / plausible `M105` temperature is **superseded**. The current dedicated pen plotter intentionally has the original printhead removed: no hotend heater, no hotend thermistor and no printhead fans. The active tool is a pen, the cylindrical magnetic/proximity sensor is the verified `z_min`, and idle hotend `T:0.00` is expected. Current operation is defined by `HARDWARE.md`, `SAFETY.md` and `GOLDEN_LIVE_FLOW.md`; do not use this historical checkpoint to block plotting on the absent thermistor.

This file names the post-incident software/workflow baseline for the Kobra 2 Neo pen plotter as understood on 2026-09-28. It remains useful as historical evidence, not as the current hardware contract.

It is a **software/workflow checkpoint**, not a statement that the physical printer is currently cleared for another plot.

## Cross-repository baseline

- `hardware-lab`: baseline before checkpoint finalization: `ff20e3f516b552be6806c25a5687bc0a7ef836ed` (`Create post-incident Kobra golden checkpoint`).
- `host-ops`: `20a27f7b930cf083c7ea998508f8b4b2f1153ea6` (`Clarify serial boundary for downstream live protocols`).
- `local-agent`: `72f0a813aafc86ff2fe77fe8e3fe05dd33e008ad` (`Document observable physical task progress`).

## Historical execution contract

The 2026-09-28 checkpoint originally used:

```text
prepared immutable G-code + report
    -> kobra-live offline revalidation
    -> SHA-256 pin when available
    -> explicit serial device
    -> M115 physical identity
    -> M105 heater-off thermal-health gate       [SUPERSEDED FOR CURRENT PEN HARDWARE]
    -> G28 X Y
    -> G28 Z
    -> pen up
    -> acknowledged artwork stream
    -> periodic M105                             [SUPERSEDED FOR CURRENT PEN HARDWARE]
    -> artwork M400
    -> final pen up
    -> final M400
    -> terminal COMPLETE_PEN_UP evidence
```

The durable parts remain: immutable job revalidation, `M115` identity, bounded homing/motion, per-command acknowledgement, structured progress and terminal pen-up evidence. The thermal-temperature gate does not apply to the intentionally headless current plotter.

## Current hardware correction

The current machine intentionally has:

- no original printhead/hotend assembly;
- no hotend heater cartridge;
- no hotend thermistor;
- no printhead fans;
- a pen/marker as the active tool;
- a cylindrical magnetic/proximity sensor as the verified `z_min` homing reference.

Therefore `M105` hotend `T:0.00` is expected and is **not a plotting blocker**. Heater commands remain forbidden. If Marlin itself emits `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or another kill state during an active transaction, the transaction still fails terminally.

## Evidence contract

A physical stage is reported only from explicit structured evidence or the terminal result. Generic `running`, PID, heartbeat age or `seconds_since_output` proves liveness only.

Expected long-job markers include:

- `PREFLIGHT_OK`
- `PRINTER_IDENTIFIED`
- `HOMING_XY_OK`
- `HOMING_Z_OK`
- `PEN_UP_OK`
- `DRAWING_STARTED`
- `DRAWING N/TOTAL`
- `COMPLETE_PEN_UP`

A firmware kill state is terminal and final pen state must be reported unknown unless later acknowledgement proves otherwise.

## Repository ownership

- `hardware-lab/projects/kobra2-neo` owns Kobra calibration, bounds, G-code validation, hardware profile and long-running Marlin protocol.
- `host-ops` owns generic host/device discovery and bounded raw serial transactions. It is not the Kobra session runner.
- `local-agent` owns binding/scheduling/watchdogs and durable run/result evidence.
- Do not assume executables installed in one repository worker are present in another worker PATH.

## Branch hygiene at audit time

- `hardware-lab`: only `main` and required `agent-control`.
- `host-ops`: only `main` and required `agent-control`.
- `local-agent`: infrastructure/state branches `chat-bridge-state` and `operator-control` remain; diverged development branches with unique commits remain; `fix/chat-bridge-grouped-turn-fallback` and `fix/chat-bridge-mixed-dom-ordering` were identified as fully contained in `main` and therefore stale. The connected GitHub control surface used for this audit did not expose branch/ref deletion, so those two refs were not falsely claimed as deleted.

## CI / validation status at checkpoint finalization

- `hardware-lab`: the checkpoint precursor `ff20e3f516b552be6806c25a5687bc0a7ef836ed` passed both the Kobra offline gate and Gitleaks.
- `local-agent`: CI passed on `72f0a813aafc86ff2fe77fe8e3fe05dd33e008ad`.
- `host-ops`: a no-write local quality audit was executed on exact main SHA `20a27f7b930cf083c7ea998508f8b4b2f1153ea6` and completed exit 0.

These are historical checkpoint facts and do not override the current hardware documentation.

## Incident result preserved by this checkpoint

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

The valid lesson from this incident is that an actual Marlin kill during execution is terminal. It is **not** evidence that the intentionally absent hotend thermistor must be restored.

## Current physical preflight

Before a physical plot:

- verify the full X/Y mechanical path is clear;
- specifically keep the printer power cable out of the rear Y-bed path;
- verify current pen/holder/Z-sensor geometry is unchanged or re-calibrate it;
- do not use nozzle-temperature controls and do not send heater commands;
- do not block on idle `T:0.00` from the intentionally absent hotend thermistor.

## Rule for future sessions

For an already prepared and approved job, the expected path is one project-local live task using `kobra-live`. Current `HARDWARE.md`, `SAFETY.md`, `WORKFLOW.md` and `GOLDEN_LIVE_FLOW.md` override this historical checkpoint wherever they differ. If a real prerequisite fails, report the exact blocker immediately and stop at that boundary. Do not spend hours silently regenerating state, bouncing between repository workers or creating speculative serial-task variants.
