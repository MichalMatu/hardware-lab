#pragma once

#include <Arduino.h>
#include <stddef.h>

namespace drivers {

/// One physical button connected to a GPIO with internal pull-up.
struct ButtonConfig {
  /// Stable name used in serial diagnostics.
  const char* name;
  /// Arduino pin alias from the active board variant.
  uint8_t pin;
};

/// Thin GPIO reader for the OLED panel button bank.
class ButtonPanel {
 public:
  /// References a static button configuration table owned by the caller.
  ButtonPanel(const ButtonConfig* buttons, size_t count);

  /// Configures all button pins as pull-up inputs.
  void begin() const;
  /// Returns the number of configured buttons.
  size_t count() const;
  /// Returns static metadata for one configured button.
  const ButtonConfig& at(size_t index) const;
  /// Reads whether one button is currently pressed.
  bool pressed(size_t index) const;

 private:
  const ButtonConfig* buttons_;
  size_t count_;
};

}  // namespace drivers
