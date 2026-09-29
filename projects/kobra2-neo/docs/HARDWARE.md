# Hardware state

## Machine

- Anycubic Kobra 2 Neo converted into a dedicated pen plotter / XY-Z motion platform.
- Stock Marlin `bugfix-2.1.x`, build Jul 28 2023.
- X/Y/Z motion is operational when firmware is not in a halted safety state.
- The original printhead / hotend assembly is intentionally removed.
- There is **no hotend heater cartridge, no hotend thermistor and no printhead fans** in the current plotting configuration.
- A disconnected hotend thermistor and an `M105` hotend reading of `T:0.00` are therefore **expected normal state**, not a hardware fault and not a plotting blocker.
- The current tool is a pen/marker in the validated printed holder.
- A cylindrical magnetic/proximity Z sensor is installed and verified as `z_min`; it is the active Z homing reference for the pen setup.
- The rear physical button near the wipe/calibration area is verified as `z_max`.

This is the current canonical hardware contract. Older notes that describe a conventional printer hotend, require a healthy hotend thermistor for plotting, or treat the removed printhead as a temporary fault are historical and must not be used as the operating contract.

## Plotter-specific implications

- Never require the removed hotend, heater, thermistor or printhead fans to be reconnected before pen plotting.
- `T:0.00` from the absent hotend thermistor is expected and must not fail plotter preflight.
- Heater commands remain forbidden in plot jobs because there is no heater hardware to control.
- Do not use the printer-screen nozzle-temperature controls during plotting. They are irrelevant to the pen tool and may cause stock Marlin to enter a thermal fault state because the hotend sensor is intentionally absent.
- If Marlin itself emits `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or another firmware halt while motion is running, treat that firmware halt as terminal for the current transaction. This is different from merely observing the expected idle `T:0.00` value.

## Verified USB / serial path

- USB bridge: QinHeng CH340, VID `0x1a86`, PID `0x7523`.
- Serial rate: 115200 baud.
- Current observed macOS device: `/dev/cu.usbserial-130`.
- `M115` reports `MACHINE_TYPE:AnycubicKobra`.
- A short serial settle interval is required by the current host workflow; `--settle 2` is the previously validated host-ops probe value.

The serial device path is not a stable identity. Another CH340 device exists in the environment, so the live executor verifies printer identity with `M115` before motion.

## Verified software limits

`M211` reported software endstops enabled with:

- X: `-5.80 .. 230.00` mm;
- Y: `-1.00 .. 230.00` mm;
- Z: `0.00 .. 250.00` mm.

These firmware limits are not the same as the safe plotting envelope of the pen tip. See `CALIBRATION.md`.

## Verified endstop / sensor mapping

Baseline with Z raised and the rear button released:

- `x_min: open`
- `y_min: open`
- `z_min: open`
- `z_max: open`
- `filament: TRIGGERED`

Physical mapping:

- X endstop -> `x_min`;
- Y endstop -> `y_min`;
- cylindrical magnetic/proximity Z sensor -> `z_min`;
- rear physical button -> `z_max`.

The cylindrical sensor was manually triggered with metal and `M119` changed `z_min` to `TRIGGERED`. The rear button was manually pressed with Z raised and `M119` changed `z_max` to `TRIGGERED`.

## Verified homing / motion evidence

- `G28 X Y` completed successfully; firmware reported X=-5.80, Y=-1.00.
- `G28 Z` completed successfully with the cylindrical magnetic/proximity sensor acting as the Z reference; the 2026-09-28 session reported X=36.00, Y=206.65, Z=2.97.
- The pen was then raised to Z=6.12 and planner completion was confirmed with `M400`.
- A full approved 10 cm artwork completed all 7615 acknowledgement-driven commands without firmware error and ended pen-up.
- On 2026-09-29 the following manual start sequence was re-verified successfully with the current pen setup:

```gcode
G28 X Y
G28 Z
G0 Z6.12 F180
G0 X90.69 Y134.14 F3000
```

- During the first 2026-09-29 retry, Y-bed travel was mechanically blocked by the printer power cable, causing the bed to reach the obstruction before `y_min`. After clearing the cable from the Y travel path, the same homing/start sequence completed normally. Treat a clear rear Y travel path, especially the power cable, as a mandatory physical preflight check before `G28 X Y`.

Homing remains an explicit operator decision even though the current Z-reference path has been physically verified.

## Historical thermal incident — interpretation for the current plotter

On 2026-09-28 a `shaft-50x20-showcase` task reached command 3300/4345 before stock Marlin emitted:

```text
Error:MINTEMP triggered, system stopped! Heater_ID: E0
```

That run established that a **firmware kill/halt message during an active transaction must be treated as terminal**. It does **not** establish that the absent hotend thermistor must be reinstalled or that idle `T:0.00` is invalid for the current pen-only machine.

The current machine intentionally has no hotend heater or thermistor. Plotter software must therefore not use an `M105` room-temperature hotend check as a prerequisite for motion. It must still fail closed if Marlin itself reports a halt/kill state while commands are being acknowledged.

## Controller identity

Exact physical MCU revision is still unverified. Do not choose a firmware-flash target from internet model assumptions alone.
