# OLED Panel Wiring

This describes the currently working black OLED panel with header labels:

```text
GND VCC SCL SDA K4 K3 K2 K1
```

## Working wiring

| OLED pin | nRF52840 alias | Purpose |
| --- | --- | --- |
| GND | GND | Ground |
| VCC | VCC / 3.3V | Display power |
| SCL | PIN_020 | I2C clock |
| SDA | PIN_022 | I2C data |
| K4 | PIN_024 | Button input |
| K3 | PIN_100 | Button input |
| K2 | PIN_011 | Button input |
| K1 | PIN_104 | Button input |

Button order in normal menu firmware is physical top-to-bottom on the side
buttons: `K1` up, `K2` down, `K3` enter, `K4` return.

The firmware leaves `PIN_017` unused/high-Z for this wiring, configures the
buttons as `INPUT_PULLUP`, and tries SSD1306 addresses `0x3C` and `0x3D`.

## Stability notes

- Disconnect USB before rewiring the panel.
- Verify `GND`, `VCC`, `SCL`, and `SDA` before powering the board.
- The OLED must be powered from `VCC / 3.3V`, not directly from a GPIO.
- Use the diagnostic module by setting `kOledDiagnosticMode = true` in
  `include/app_config.h`. Commands over serial:
  - `l` reads SDA/SCL levels.
  - `s` scans I2C.
  - `r` sends I2C recovery pulses.
  - `b` prints button states.
