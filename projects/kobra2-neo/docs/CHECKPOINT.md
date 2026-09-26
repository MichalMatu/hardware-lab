# Migrated project checkpoint

Source: `MichalMatu/host-ops/docs/plans/KOBRA2_NEO_CHECKPOINT.md` at project bootstrap. Future Kobra checkpoints live in this project.

# Kobra 2 Neo live-device checkpoint

**Status:** operational checkpoint for the current Anycubic Kobra 2 Neo conversion work. This document records live evidence and next operator decisions; it does not make Kobra-specific behavior part of the generic `host-ops` runtime.

## Current hardware state

- The original print head assembly has been removed, including the original Z-distance/probe sensor.
- The carriage and motion system remain connected and controllable.
- Do **not** run Z homing, mesh leveling or probe-dependent routines in this state. The original probe is absent, so the stock Z-homing contract is no longer valid.
- The preferred direction is a modular tool-carriage interface rather than restoring the stock hotend as the permanent tool.

If a new mount must be printed on this same printer, the stock head/probe may be reinstalled temporarily for that print. If the mount can be fabricated elsewhere, keep the machine headless and continue with the modular carriage.

## Verified host connection

Live macOS evidence established the working path as:

- USB transport: USB-A -> USB-C through the current hub;
- USB identity: QinHeng/CH340 `VID 0x1a86`, `PID 0x7523`;
- serial device: `/dev/cu.usbserial-1120`;
- macOS driver path works without an additional vendor driver;
- USB-C -> USB-C did not enumerate this printer in the tested setup.

## Verified firmware/serial contract

The stock controller responds at `115200` baud.

`M115` live evidence:

- firmware: `Marlin bugfix-2.1.x`;
- build: `Jul 28 2023 14:19:17`;
- protocol: `1.0`;
- machine type string: `AnycubicKobra`;
- one extruder;
- EEPROM, SD card, auto leveling, runout and Z-probe capabilities are compiled in.

Opening the CH340 serial port requires a short settle interval before the first request. `hostops serial transact` now exposes `--settle` for this generic transport need. `--settle 2` was validated against this printer with `M115`.

## Verified motion/safety state

Live `M211` reported software endstops enabled:

- X: `-5.80 .. 230.00` mm;
- Y: `-1.00 .. 230.00` mm;
- Z: `0.00 .. 250.00` mm.

The stock Anycubic configuration uses MIN-direction homing for X, Y and Z. Because the current toolhead/probe is removed, do not infer the physical machine coordinate from `M114` after opening the serial port and do not use `G28 Z` until a new Z-reference strategy exists.

With the carriage physically positioned near the middle of travel and with operator-confirmed clearance, the following live motion checks completed successfully:

- Z `+20 / -20` mm;
- X `+20 / -20` mm;
- Y `+20 / -20` mm;
- a 20 x 20 mm XY square;
- coordinated XY diagonal `+20,+20` and return `-20,-20`;
- `M400` used after every motion segment to prove planner completion;
- every test returned to the starting logical position;
- no heaters, extrusion or firmware flashing were used.

This proves the existing stock Marlin firmware is sufficient for bounded plotter-style XY/Z motion over USB.

## Firmware decision

Do **not** flash Klipper or another firmware yet.

Reasons:

1. stock Marlin already provides the motion control required for the first plotter/tool-carriage phase;
2. the physical MCU revision is not yet proven for this exact controller board;
3. the original Z probe has been removed, so a new homing/reference design is needed before firmware migration adds value;
4. firmware flashing would add recovery risk without solving the current mechanical problem.

Revisit Klipper when a concrete requirement appears, for example reusable tool macros, external paste/clay extrusion control, richer host-side coordination, or a redesigned Z-reference system. Before flashing, identify the exact board/MCU from the physical controller and preserve a recovery path.

## Mechanical direction

Treat the Kobra as a generic three-axis motion platform with interchangeable tools.

Target carriage architecture:

```text
Kobra X carriage
  -> common rigid adapter plate
      -> pen/marker module
      -> clay/paste extrusion module
      -> future tool modules
```

The common adapter should provide:

- repeatable mechanical datum;
- a small set of fixed mounting holes or a keyed quick-change interface;
- enough Z adjustment for different tools;
- low mass close to the original carriage plane;
- cable/hosing strain relief independent of the tool;
- room for a future Z-reference/probe module that is not tied to the original hotend.

For clay/paste printing, prefer keeping the heavy material reservoir off the moving carriage when possible and feed material through a hose to a lightweight nozzle/toolhead. The exact extrusion mechanism is a later module decision, not a reason to restore the stock hotend permanently.

## Next phase

1. Measure and document the bare carriage mounting geometry after removing the stock head.
2. Choose the common adapter datum and fastener pattern.
3. Design/print the first adapter plus a marker holder.
4. Add a new Z-reference strategy before any homing or automatic Z routines:
   - dedicated switch/probe on the modular carriage, or
   - another repeatable external reference with an explicit operator workflow.
5. Calibrate marker pen-up/pen-down heights manually with small bounded moves.
6. Perform the first real paper plot using stock Marlin.
7. Only then decide whether a clay/paste module creates a real requirement for firmware migration.

## host-ops boundary

Keep `host-ops` generic:

- serial transport, discovery, bounds and evidence belong here;
- Marlin command sequences, plot generation, tool-change policy, clay extrusion policy and printer-specific calibration belong in the device/project workflow outside generic capabilities;
- do not add a generic serial-sequence abstraction until repeated workflows establish stable semantics.
