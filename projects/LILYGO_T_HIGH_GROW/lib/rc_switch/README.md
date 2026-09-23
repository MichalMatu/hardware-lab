# rc-switch (local copy)

This directory contains the trimmed 2.6.4 release of `sui77/rc-switch`.
Only the core sources needed for ESP32 use are kept; examples and IDE
artifacts have been removed to keep the firmware tree lean.

- `src/RCSwitch.h` / `src/RCSwitch.cpp` — full transmitter/receiver logic.
- `library.json` — upstream metadata preserved for provenance.

Licensing: the upstream project is LGPL 2.1+. When making changes or
redistributing binaries, ensure compliance with the original license.
