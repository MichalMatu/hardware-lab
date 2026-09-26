# Hardware state

## Machine

- Anycubic Kobra 2 Neo.
- Original print head removed.
- Original Z-distance/probe sensor removed with the head.
- X/Y/Z motion system remains operational.

## Verified USB path

- Mac host connection: USB-A -> USB-C through the current hub.
- USB bridge: QinHeng CH340, VID `0x1a86`, PID `0x7523`.
- macOS serial device observed: `/dev/cu.usbserial-1120`.
- Stock firmware serial rate: 115200 baud.
- USB-C -> USB-C did not enumerate in the tested setup.

## Verified firmware

- Stock Marlin `bugfix-2.1.x`, build Jul 28 2023.
- Software endstops observed enabled.
- Verified bounded relative motion on X, Y and Z, a 20 x 20 mm XY square and coordinated XY diagonal motion.

## Controller identity

Exact physical MCU revision is still unverified. Do not choose a Klipper/firmware target from internet model assumptions alone.
