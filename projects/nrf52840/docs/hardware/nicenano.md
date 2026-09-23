# nRF52840 SuperMini / nice!nano Notes

## Bootloader

To enter bootloader mode without a reset button, short `RST` to `GND` twice
quickly:

```text
RST -> GND, release, RST -> GND, release
```

When bootloader mode works, macOS should show:

```text
/dev/cu.usbmodem*
/Volumes/NICENANO
```

Do not use `/dev/cu.usbserial-*` unless you have verified that it belongs to
this board. In this workspace `/dev/cu.usbserial-120` was an ESP32/CH340, not
the nRF52840.

## Pins

The local variant exposes Nordic-style aliases such as:

```cpp
PIN_015  // P0.15, default status LED in this starter
PIN_104  // P1.04, SDA
PIN_106  // P1.06, SCL
PIN_111  // P1.11, SCK
PIN_113  // P1.13, MOSI
PIN_115  // P1.15, MISO
```

The current OLED test remaps I2C in firmware:

```cpp
Wire.setPins(PIN_022, PIN_020);  // SDA, SCL
```

See [oled-panel.md](oled-panel.md) for the working OLED wiring.
