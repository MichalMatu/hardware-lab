# Golden live flow

Status: canonical operator flow for the current dedicated Kobra 2 Neo pen plotter.

This document is the default path for every approved Kobra 2 Neo pen plot. Do not rebuild an ad-hoc serial streamer in a task payload when this path is available.

## Canonical hardware assumption

The current machine is **not a complete 3D-printer toolhead configuration**:

- the original printhead / hotend is removed;
- there is no hotend heater cartridge;
- there is no hotend thermistor;
- there are no printhead fans;
- the active tool is a pen/marker;
- the cylindrical magnetic/proximity sensor is the verified `z_min` reference used for Z homing;
- idle `M105` hotend `T:0.00` is expected because the sensor is physically absent.

Do not block a pen plot merely because the absent hotend reports `T:0.00`, and do not instruct the operator to reconnect the removed thermistor as a prerequisite for plotting. Heater commands are forbidden because there is no heater hardware. A firmware-reported kill/halt during execution remains a terminal condition.

## Ownership boundary

- `hardware-lab/projects/kobra2-neo` owns Kobra identity, calibration, G-code policy, plotter hardware profile and the Marlin streaming protocol.
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

The runner revalidates the immutable artifact before opening serial, verifies the printer with `M115`, applies the current headless pen-plotter profile, homes XY then Z, raises the pen, streams the complete approved artwork with Marlin acknowledgement after every command, waits for completion and finishes pen-up.

A separate host-ops probe is not required merely because a new chat started. Reuse the known port when the host/cabling session has not changed; `kobra-live` still proves physical printer identity with `M115` before motion. Use host-ops serial discovery only when the port is unknown, changed, ambiguous or the direct identity check fails.

## Required evidence gates

1. **No competing live task.** Read the hardware-lab daemon/run state first. If a physical task is already running, do not enqueue another serial task or probe the same port.
2. **Physical travel clear.** Before XY homing, verify the bed and carriage can reach their endstops. In particular, keep the printer power cable out of the rear Y-bed path; it caused a verified mechanical homing obstruction on 2026-09-29.
3. **Artifact preflight.** G-code and report must exist. Safety must be valid; artwork may contain only `G90`, `G0`, `G1`, `M400`; no `G28`, heaters, extrusion, relative mode or out-of-envelope motion; Z may use only the calibrated pen-up/down values.
4. **Immutable identity.** Prefer pinning the approved G-code SHA-256 with `--expect-sha256`. The Local Agent task must also run from the intended repository revision.
5. **Physical identity.** `M115` must identify Anycubic Kobra before motion.
6. **Headless plotter profile.** The missing hotend, heater, thermistor and printhead fans are intentional. `T:0.00` from the absent hotend thermistor is expected and is not a pre-motion blocker. No heater command may be sent.
7. **Explicit live approval.** Approval covers one transaction: XY home -> Z home -> pen up -> first XY -> artwork -> `M400` -> final pen up.
8. **Final proof.** Success means a terminal `RESULT:KOBRA_LIVE_COMPLETE_PEN_UP ...` or equivalent final task evidence. Liveness alone is not completion.

## Verified homing / start

The current pen setup has re-verified this sequence:

```gcode
G28 X Y
G28 Z
G0 Z6.12 F180
G0 X90.69 Y134.14 F3000
```

The cylindrical magnetic/proximity Z sensor is the active `z_min` reference. Revalidate calibration after changing pen length, holder geometry, paper thickness or the Z-sensor geometry.

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

A firmware `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or equivalent kill state emitted during command execution must produce a terminal failure such as `FIRMWARE_HALTED_FINAL_PEN_STATE_UNKNOWN`. This does not make idle `T:0.00` a fault; the distinction is between an expected absent sensor and an actual firmware halt.

## Fail-fast communication rule

There must be no silent multi-hour recovery loop.

- Any validation, worker-capability, serial identity, firmware, timeout, resend or repository mismatch is reported immediately as the current blocker.
- Do not report the intentionally absent hotend thermistor or idle `T:0.00` as a blocker for this hardware profile.
- A failed attempt must not be replaced by repeated speculative task variants. Diagnose the exact failed boundary first.
- If a command/tool expected in one worker is missing, report that worker mismatch explicitly; do not silently bounce between repositories.
- Long live tasks must emit structured progress at least once per Local Agent heartbeat window.
- If only liveness evidence exists, say exactly that. Do not infer homing, drawing or completion from `seconds_since_output`.

## Recovery

If the runner fails before physical identity or homing, it stops without deliberate plot motion beyond what has already been acknowledged.

For a non-fatal streaming/transport failure after successful Z homing, the runner makes one bounded best-effort absolute pen-up attempt and reports whether that recovery was acknowledged.

For `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or another Marlin kill state **actually emitted during execution**, further motion is not considered reliable. The runner does not pretend to recover with repeated movement commands; it reports final pen state unknown. The machine must be physically inspected/reset before another transaction.

Never open a second serial session to diagnose the printer while the first live task is still running.

## 2026-09-28 thermal incident — corrected interpretation

The `shaft-50x20-showcase` task passed identity and both homing stages, started drawing and reached command 3300/4345 before Marlin emitted `Error:MINTEMP triggered, system stopped! Heater_ID: E0`. The task failed after 801.199 seconds and did not confirm final pen-up.

The durable lesson is: **a firmware kill during a live transaction is terminal**. It is not evidence that the deliberately removed thermistor must be restored, and it must not create an `M105` room-temperature gate for the current pen-only hardware profile. The operator also reported that interacting with nozzle-temperature controls preceded the thermal incident; those controls must not be used for the headless pen tool.

## Golden checkpoint criteria

A Kobra plotting baseline may be called `golden` only when:

- `main` is clean and CI passes;
- only `main` plus the required Local Agent control branch remain in hardware-lab;
- current pen calibration, bounds and headless hardware profile are documented;
- the committed runner passes unit/preflight tests;
- the most recent physical live result is recorded exactly, including failures and unknown final state;
- cross-repo responsibilities above remain unchanged.
