# Historical bootstrap checkpoint

> **Superseded for current operation.** This file preserves the earlier headless-machine checkpoint migrated from `host-ops`. The current pen holder, restored/available cylindrical `z_min` sensor, successful current Z homing evidence and calibrated plotter envelope were established later. Use `HARDWARE.md`, `CALIBRATION.md`, `SAFETY.md` and `WORKFLOW.md` for current operation.

Source: `MichalMatu/host-ops/docs/plans/KOBRA2_NEO_CHECKPOINT.md` at project bootstrap.

## Historical hardware state

At this checkpoint the original print head assembly and its Z-distance/probe sensor had been removed. The carriage and motion system remained controllable, so the safe rule at that time was to avoid Z homing, mesh leveling and probe-dependent routines until a valid Z-reference strategy returned.

This historical restriction was correct for that machine state but is no longer the current hardware contract.

## Historical verified host connection

- USB transport: USB-A -> USB-C through the then-current hub;
- USB identity: QinHeng/CH340 `VID 0x1a86`, `PID 0x7523`;
- serial device observed then: `/dev/cu.usbserial-1120`;
- serial rate: 115200 baud;
- USB-C -> USB-C did not enumerate in the tested setup.

The device path was an observation, not a stable identity.

## Historical firmware/serial evidence

`M115` showed:

- firmware: `Marlin bugfix-2.1.x`;
- build: `Jul 28 2023 14:19:17`;
- protocol: `1.0`;
- machine type: `AnycubicKobra`;
- EEPROM, SD card, auto leveling, runout and Z-probe capabilities compiled in.

Opening the CH340 serial port required a short settle interval; `--settle 2` was validated.

## Historical motion evidence

`M211` reported software endstops enabled:

- X: `-5.80 .. 230.00` mm;
- Y: `-1.00 .. 230.00` mm;
- Z: `0.00 .. 250.00` mm.

With operator-confirmed clearance in the headless state, bounded relative checks completed successfully on X/Y/Z, including a 20 x 20 mm XY square and coordinated XY diagonal. `M400` was used to prove planner completion. No heaters, extrusion or firmware flashing were used.

## Historical design direction

The project direction was to treat the Kobra as a reusable motion platform with interchangeable tools rather than immediately changing firmware. That architectural direction remains useful, but the current pen holder is now a working calibrated tool and should be preserved as the baseline while future modular mechanics are evaluated.

## Firmware decision retained

Do not flash Klipper or another firmware without a concrete requirement, exact controller/MCU identification and a recovery path. Stock Marlin currently satisfies the pen-plotter motion requirements.

## Boundary retained

Keep `host-ops` generic. Kobra-specific calibration, G-code/tool policy and mechanical design belong in this project. Generic serial/device capabilities may remain external.
