# Golden live flow

Status: canonical operator flow after the 2026-09-28 slow-start incident.

This document is the default path for every approved Kobra 2 Neo pen plot. Do not rebuild an ad-hoc serial streamer in a task payload when this path is available.

## Ownership boundary

- `hardware-lab/projects/kobra2-neo` owns Kobra identity, calibration, G-code policy, preflight and the Marlin streaming protocol.
- `host-ops` is only a generic host/device capability layer. Use it to enumerate serial devices or perform a small bounded raw probe when discovery is actually needed.
- Local Agent owns repository binding, scheduling, timeout/watchdog state and durable task evidence.
- Do not assume the `hostops` executable exists in the `hardware-lab` worker PATH. Do not make live plotting depend on that assumption.

## Fast path

When the artwork and report already exist and operator approval covers the complete transaction, use one hardware-lab task:

```bash
cd projects/kobra2-neo
uv run kobra-live \
  samples/gcode/JOB.gcode \
  --report samples/gcode/JOB.report.json \
  --port /dev/cu.usbserial-130 \
  --expect-sha256 EXPECTED_SHA256
```

The runner itself revalidates the immutable artifact before opening serial, verifies the printer with `M115`, homes XY then Z, raises the pen, streams the complete approved artwork with Marlin acknowledgement after every command, waits for completion and finishes pen-up.

A separate host-ops probe is not required merely because a new chat started. Reuse the known port when the host/cabling session has not changed; `kobra-live` still proves physical printer identity with `M115` before motion. Use `hostops macos serial` / `hostops serial transact ... M115 ...` only when the port is unknown, changed, ambiguous or the direct identity check fails.

## Required evidence gates

1. **No competing live task.** Read the hardware-lab daemon/run state first. If a physical task is already running, do not enqueue another serial task or probe the same port.
2. **Artifact preflight.** G-code and report must exist. Safety must be valid; artwork may contain only `G90`, `G0`, `G1`, `M400`; no `G28`, heaters, extrusion, relative mode or out-of-envelope motion; Z may use only the calibrated pen-up/down values.
3. **Immutable identity.** Prefer pinning the approved G-code SHA-256 with `--expect-sha256`. The Local Agent task must also run from the intended repository revision.
4. **Physical identity.** `M115` must identify Anycubic Kobra before any motion.
5. **Explicit live approval.** Approval covers one transaction: XY home -> Z home -> pen up -> first XY -> artwork -> `M400` -> final pen up.
6. **Final proof.** Success means a terminal `RESULT:KOBRA_LIVE_COMPLETE_PEN_UP ...` or equivalent final task evidence. Liveness alone is not completion.

## Observable progress contract

`kobra-live` emits Local Agent native progress markers:

```text
[AGENT_PROGRESS] {"stage_name":"kobra-live",...}
```

Expected messages include:

```text
PREFLIGHT_OK
PRINTER_IDENTIFIED
HOMING_XY_OK
HOMING_Z_OK
PEN_UP_OK
DRAWING_STARTED
DRAWING N/TOTAL
COMPLETE_PEN_UP
```

These messages populate Local Agent `last_progress_message`. An operator or assistant must never state that a stage passed unless the corresponding progress/result evidence exists.

## Fail-fast communication rule

There must be no silent multi-hour recovery loop.

- Any validation, worker-capability, serial identity, firmware, timeout, resend or repository mismatch is reported immediately as the current blocker.
- A failed attempt must not be replaced by repeated speculative task variants. Diagnose the exact failed boundary first.
- If a command/tool expected in one worker is missing, report that worker mismatch explicitly; do not silently bounce between repositories.
- Long live tasks must emit structured progress at least once per Local Agent heartbeat window. The default stream marker every 100 commands satisfies this for normal plots.
- If only liveness evidence exists, say exactly that. Do not infer homing, drawing or completion from `seconds_since_output`.

## Recovery

If the runner fails before physical identity or homing, it stops without motion beyond what has already been acknowledged.

If streaming fails after successful Z homing, the runner makes a bounded best-effort absolute pen-up attempt and reports that recovery attempt. A firmware error, resend request or acknowledgement timeout terminates the transaction; it is not retried blindly.

Never open a second serial session to diagnose the printer while the first live task is still running.

## Golden checkpoint criteria

A Kobra plotting baseline may be called `golden` only when:

- `main` is clean and CI passes;
- only `main` plus the required Local Agent control branch remain in hardware-lab;
- current calibration and bounds are documented;
- the committed runner passes unit/preflight tests;
- the most recent physical live job result is recorded without overstating unobserved stages;
- cross-repo responsibilities above remain unchanged.
