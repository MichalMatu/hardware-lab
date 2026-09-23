#pragma once

#include <Arduino.h>

namespace app_config {

/// Built-in status LED pin from the active Arduino board variant.
constexpr uint8_t kStatusLedPin = LED_BUILTIN;
/// Logical GPIO state that turns the built-in LED on for this board.
constexpr uint8_t kStatusLedOnState = LED_STATE_ON;
/// Period for the firmware heartbeat LED.
constexpr uint32_t kStatusBlinkIntervalMs = 1000UL;
/// USB CDC serial speed used by diagnostics and placeholders.
constexpr uint32_t kSerialBaudrate = 115200UL;
/// Debounce time for panel button click detection.
constexpr uint32_t kButtonDebounceMs = 20UL;

/// Legacy GPIO power pin kept high-Z because the OLED is powered from VCC.
constexpr uint8_t kOledPowerPin = PIN_017;
/// False when the display is powered directly from board VCC / 3.3 V.
constexpr bool kOledUsesGpioPower = false;
/// OLED I2C clock pin.
constexpr uint8_t kOledSclPin = PIN_020;
/// OLED I2C data pin.
constexpr uint8_t kOledSdaPin = PIN_022;
/// OLED pixel width.
constexpr uint16_t kOledWidth = 128;
/// OLED pixel height.
constexpr uint16_t kOledHeight = 64;
/// Minimum interval for status screen refreshes.
constexpr uint32_t kOledUpdateIntervalMs = 250UL;
/// Periodic menu redraw interval used as a low-cost display refresh guard.
constexpr uint32_t kMenuRefreshIntervalMs = 1000UL;
/// Diagnostic heartbeat and I2C scan reporting interval.
constexpr uint32_t kOledScanIntervalMs = 3000UL;
/// Conservative I2C speed for the soldered OLED panel wiring.
constexpr uint32_t kI2cClockHz = 100000UL;
/// Compile-time switch for serial-only OLED/I2C diagnostics.
constexpr bool kOledDiagnosticMode = false;

/// OLED header K4 button pin.
constexpr uint8_t kButtonK4Pin = PIN_024;
/// OLED header K3 button pin.
constexpr uint8_t kButtonK3Pin = PIN_100;
/// OLED header K2 button pin.
constexpr uint8_t kButtonK2Pin = PIN_011;
/// OLED header K1 button pin.
constexpr uint8_t kButtonK1Pin = PIN_104;

}  // namespace app_config
