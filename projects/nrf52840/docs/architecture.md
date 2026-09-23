# Firmware Architecture

The firmware is split into small modules so hardware access, application state,
and project configuration can evolve independently.

## Runtime Flow

`src/main.cpp` owns Arduino `setup()` and `loop()` only. It wires together:

- `drivers::ButtonPanel` for raw GPIO button reads.
- `app::ButtonInput` for debounced press events.
- `drivers::OledPanel` for SSD1306 startup, recovery, and menu rendering.
- `app::DeviceMenu` for the current menu state.
- `app::StatusLed` for the heartbeat LED.
- `app::OledDiagnostics` for serial-only wiring recovery.

Normal mode initializes the OLED first. If the display is ready, the firmware
starts the menu. If OLED startup fails, the firmware falls back to serial
diagnostics instead of blocking the main loop.

## Ownership Boundaries

- `include/app_config.h` is the source of truth for pins, timing, and diagnostic
  compile-time switches.
- `src/drivers/` talks directly to GPIO, I2C, and display libraries.
- `src/app/` owns state machines and feature coordination.
- `docs/hardware/` records physical wiring assumptions.
- `vendor/` is ignored research material and must not be required to build.

## Menu And Future HID

`app::DeviceMenu` currently emits placeholder actions over USB serial. The next
feature should add a separate BLE HID module instead of placing BLE calls inside
the menu or OLED driver.

Expected direction:

- `DeviceMenu` keeps selecting device modes and actions.
- A future `BleHidController` owns Bluefruit setup, advertising, connection
  state, keyboard reports, mouse reports, and media keys.
- `main.cpp` maps a selected menu action to one method on the HID controller.

This keeps display rendering, button input, and BLE behavior testable as
separate surfaces.
