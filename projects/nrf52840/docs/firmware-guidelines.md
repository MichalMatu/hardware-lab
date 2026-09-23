# Firmware Guidelines

These rules keep the nRF52840 firmware easy to extend without wasting RAM,
flash, GPIO current, or battery life.

## Architecture

- `src/main.cpp` should only coordinate Arduino `setup()` and `loop()`.
- `src/drivers/` owns direct hardware access such as OLED, buttons, sensors,
  I2C, SPI, and status LEDs.
- `src/app/` owns application state machines and cross-driver coordination.
- `src/features/` can hold optional features when they become independent
  enough to disable or replace.
- `include/` holds public project headers and compile-time configuration.

## Preferred patterns

- Use `begin()` for one-time hardware initialization.
- Use non-blocking `update(now)` methods for repeated work.
- Store static configuration as `constexpr` values.
- Keep pin names symbolic, for example `OLED_SDA_PIN`, not raw literals.
- Keep display updates rate-limited.
- Keep BLE, USB, display, and sensor code behind small interfaces.

## Anti-patterns

- Long `delay()` calls in `loop()`.
- Arduino `String` allocation in recurring code.
- Heap allocation after startup.
- Recursion, exceptions, RTTI, and large standard containers on the MCU.
- Duplicate pin assignments across files.
- Silent dependence on `vendor/` examples or generated `.pio/` files.
- Serial-only behavior where the firmware cannot run headless.

## Resource policy

The nRF52840 has meaningful headroom, but the board is still a constrained MCU.
Nordic documents the nRF52840 as having 1 MB flash and 256 KB RAM, USB full
speed, two I2C-compatible controllers, and flexible power management.

Use these project rules:

- Treat `pio run` memory output as a required review signal.
- Re-run `scripts/size-report.sh` before merging features that add BLE,
  graphics, buffers, filesystems, crypto, or protocol stacks.
- Prefer fixed-size buffers with documented maximum lengths.
- Avoid keeping duplicate display buffers or scan result arrays.
- Disable or compile-gate diagnostics that are not needed in production.
- For low-power work, remove unnecessary periodic display/serial updates first.

## Current OLED notes

The current OLED firmware uses one SSD1306 framebuffer. A 128x64 monochrome
buffer costs 1024 bytes before library overhead. This is acceptable for the
current test firmware, but avoid adding additional full-screen buffers unless a
feature requires it.

The OLED is powered from board `VCC / 3.3V`. Do not power the display directly
from a GPIO. If switched display power is needed later, use a proper transistor
or MOSFET stage and keep the firmware pin as an enable signal only.

The I2C/OLED diagnostic path is intentionally kept outside the display driver.
Enable it with `kOledDiagnosticMode` when wiring needs to be checked without
initializing the SSD1306 framebuffer.

## References

- C++ Core Guidelines:
  https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
- nRF52840 product specification:
  https://docs.nordicsemi.com/r/bundle/ps_nrf52840/page/keyfeatures_html5.html
