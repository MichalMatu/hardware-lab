# OLED Menu

The normal firmware uses the OLED and four panel buttons as a small device-mode
menu. BLE HID actions are placeholders for now; selecting an action writes the
intended HID command to USB serial.

## Button map

Physical order is top to bottom on the OLED button column.

| Physical button | Header pin | Function |
| --- | --- | --- |
| 1 | K1 / PIN_104 | Up / previous item |
| 2 | K2 / PIN_011 | Down / next item |
| 3 | K3 / PIN_100 | Select / run action |
| 4 | K4 / PIN_024 | Back |

The firmware does not render button hints on the OLED because the panel already
has printed labels next to the buttons.

## Current menu

```text
Device type
|-- Keyboard
|   |-- Copy
|   |-- Paste
|   |-- Enter
|   `-- Escape
|-- Mouse
|   |-- Left click
|   |-- Right click
|   |-- Scroll up
|   `-- Scroll down
|-- Media
|   |-- Play/Pause
|   |-- Volume +
|   |-- Volume -
|   `-- Next track
|-- Gamepad
|   |-- Button A
|   |-- Button B
|   |-- D-pad up
|   `-- D-pad down
`-- Diagnostics
    |-- I2C levels
    |-- I2C scan
    |-- Button test
    `-- BLE status
```

## State model

The menu has two screens:

- Device list: ENTER opens the selected device type.
- Action list: ENTER runs the selected action placeholder, RETURN goes back to
  the device list.

UP and DOWN wrap around at the top and bottom of the current screen. The OLED
renders up to five visible rows and scrolls the list window when needed.

The next implementation step is to replace the serial placeholders with a
`BleHidController` module based on Bluefruit `BLEHidAdafruit`.
