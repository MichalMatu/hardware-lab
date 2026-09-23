#pragma once

#include <Arduino.h>

namespace app {

/// Non-blocking heartbeat LED driven from Arduino loop().
class StatusLed {
 public:
  /// Creates a status LED controller for a pin and active logic level.
  StatusLed(uint8_t pin, uint8_t on_state, uint32_t interval_ms);

  /// Configures the LED pin and starts from the off state.
  void begin();
  /// Toggles the LED when the configured interval elapsed.
  bool update(uint32_t now_ms);
  /// Returns the last requested logical LED state.
  bool enabled() const;

 private:
  void set(bool enabled);
  uint8_t off_state() const;

  uint8_t pin_;
  uint8_t on_state_;
  uint32_t interval_ms_;
  uint32_t last_toggle_ms_;
  bool enabled_;
};

}  // namespace app
