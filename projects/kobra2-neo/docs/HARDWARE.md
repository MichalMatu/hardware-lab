# Hardware state

This file describes the current physical machine, not historical printer assumptions.

## Machine

- Anycubic Kobra 2 Neo converted into a dedicated pen plotter / XY-Z motion platform.
- Stock Marlin `bugfix-2.1.x`, build Jul 28 2023.
- Original printhead / hotend assembly intentionally removed.
- No hotend heater cartridge.
- No hotend thermistor.
- No printhead fans.
- Current tool: pen/marker in the validated holder.
- Cylindrical magnetic/proximity sensor is verified as `z_min` and is the active Z homing reference.
- Rear physical button is verified as `z_max`.

Older assumptions that require a conventional hotend/thermistor for pen plotting are superseded.

## Thermal implications

- Idle `M105` hotend `T:0.00` is expected because the thermistor is intentionally absent.
- `T:0.00` by itself must not be treated as a plotting blocker.
- Heater commands are forbidden because there is no heater hardware.
- Do not use nozzle-temperature controls on the printer UI during pen plotting.
- If Marlin actually emits `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or equivalent during an active transaction, that transaction is terminal.

The distinction is simple: expected absent-sensor reading is normal; an actual firmware halt is not.

## USB / serial identity

- USB bridge: QinHeng CH340, VID `0x1a86`, PID `0x7523`.
- Serial rate: 115200 baud.
- `/dev/cu.usbserial-*` path is not stable identity.
- Another CH340-class device may be present.
- `M115` must identify `AnycubicKobra` before deliberate plot motion.

Do not use a remembered port path as the only printer identity check.

## Firmware software limits

Previously observed `M211` software limits:

```text
X: -5.80 .. 230.00 mm
Y: -1.00 .. 230.00 mm
Z:  0.00 .. 250.00 mm
```

These are firmware limits, not the safe pen-tip drawing envelope. Use `CALIBRATION.md` and the active profile for plotting limits.

## Endstop / sensor mapping

Verified mapping:

```text
X endstop -> x_min
Y endstop -> y_min
cylindrical magnetic/proximity Z sensor -> z_min
rear physical button -> z_max
```

The cylindrical sensor was manually triggered with metal and `M119` changed `z_min` to `TRIGGERED`. The rear button was manually pressed and `M119` changed `z_max` to `TRIGGERED`.

## Homing evidence

Known-good physical behavior:

- `G28 X Y` has completed successfully.
- `G28 Z` has completed successfully using the cylindrical `z_min` sensor.
- The 2026-09-28 session reported Z=2.97 at the established Z reference/contact state.
- A complete pen artwork and the 2026-09-29 map plot both demonstrated usable XY/Z pen mechanics.

The old manual start examples using `Z6.12 F180` are historical and must not be copied into new jobs. Current tuned values are defined in `config/kobra2_neo_pen.toml` and `CALIBRATION.md`.

## Mechanical preflight

Before `G28 X Y`, clear the complete bed/carriage travel path. On 2026-09-29 the printer power cable physically blocked rear Y travel before the endstop. After moving the cable, the same homing operation worked normally.

Treat the rear Y-bed path and power cable as a mandatory physical preflight check.

## Controller identity

Exact physical MCU revision is still unverified. Do not choose a firmware-flash target from internet model assumptions alone.