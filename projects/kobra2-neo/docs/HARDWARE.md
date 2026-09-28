# Hardware state

## Machine

- Anycubic Kobra 2 Neo.
- Stock Marlin `bugfix-2.1.x`, build Jul 28 2023.
- X/Y/Z motion is operational.
- Current plotting setup has a working pen holder and a cylindrical Z sensor verified as `z_min`.
- Rear physical button near the wipe/calibration area is verified as `z_max`.

Earlier project bootstrap notes described a temporary headless state with the original probe removed. That state is historical and must not be used as the current operating contract.

## Verified USB / serial path

- USB bridge: QinHeng CH340, VID `0x1a86`, PID `0x7523`.
- Serial rate: 115200 baud.
- Current observed macOS device: `/dev/cu.usbserial-130`.
- `M115` reports `MACHINE_TYPE:AnycubicKobra`.
- A short serial settle interval is required by the current host workflow; `--settle 2` is the validated value.

The serial device path is not a stable identity. Another CH340 device exists in the environment, so use `M115` when printer identity is uncertain.

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
- cylindrical proximity/level sensor -> `z_min`;
- rear physical button -> `z_max`.

The cylindrical sensor was manually triggered with metal and `M119` changed `z_min` to `TRIGGERED`. The rear button was manually pressed with Z raised and `M119` changed `z_max` to `TRIGGERED`.

## Verified homing / motion evidence

- `G28 X Y` completed successfully; firmware reported X=-5.80, Y=-1.00.
- `G28 Z` completed successfully with the cylindrical sensor acting as the Z reference; the 2026-09-28 session reported X=36.00, Y=206.65, Z=2.97.
- The pen was then raised to Z=6.12 and planner completion was confirmed with `M400`.
- A full approved 10 cm artwork subsequently completed all 7615 acknowledgement-driven commands without firmware error and ended pen-up.

Homing remains an explicit operator decision even though the current Z-reference path has been physically verified.

## Controller identity

Exact physical MCU revision is still unverified. Do not choose a firmware-flash target from internet model assumptions alone.
