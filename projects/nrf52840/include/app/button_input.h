#pragma once

#include <Arduino.h>
#include <stddef.h>

#include "drivers/button_panel.h"

namespace app {

/// Debounced button click event emitted on the press edge.
struct ButtonEvent {
  /// Zero-based index in the active drivers::ButtonPanel.
  size_t index;
};

/// Converts raw button GPIO reads into debounced click events.
class ButtonInput {
 public:
  /// Creates a debouncer with a fixed debounce window in milliseconds.
  explicit ButtonInput(uint32_t debounce_ms);

  /// Samples the initial button state and arms the debouncer.
  void begin(const drivers::ButtonPanel& buttons, uint32_t now_ms);
  /// Polls all configured buttons and writes one press event when available.
  bool poll(uint32_t now_ms, const drivers::ButtonPanel& buttons, ButtonEvent* event);

 private:
  static constexpr size_t kMaxButtons = 8;

  uint32_t debounce_ms_;
  bool raw_pressed_[kMaxButtons];
  bool stable_pressed_[kMaxButtons];
  uint32_t raw_changed_ms_[kMaxButtons];
  size_t count_;
  bool initialized_;
};

}  // namespace app
