#include "app/button_input.h"

namespace app {

ButtonInput::ButtonInput(uint32_t debounce_ms)
    : debounce_ms_(debounce_ms), count_(0), initialized_(false) {
  for (size_t index = 0; index < kMaxButtons; index++) {
    raw_pressed_[index] = false;
    stable_pressed_[index] = false;
    raw_changed_ms_[index] = 0;
  }
}

void ButtonInput::begin(const drivers::ButtonPanel& buttons, uint32_t now_ms) {
  const size_t button_count = buttons.count();
  const size_t limited_count = button_count < kMaxButtons ? button_count : kMaxButtons;
  count_ = limited_count;

  // cppcheck-suppress knownConditionTrueFalse
  for (size_t index = 0; index < limited_count; index++) {
    const bool pressed = buttons.pressed(index);
    raw_pressed_[index] = pressed;
    stable_pressed_[index] = pressed;
    raw_changed_ms_[index] = now_ms;
  }

  initialized_ = true;
}

bool ButtonInput::poll(uint32_t now_ms, const drivers::ButtonPanel& buttons, ButtonEvent* event) {
  if (!initialized_) {
    begin(buttons, now_ms);
    return false;
  }

  for (size_t index = 0; index < count_; index++) {
    const bool pressed = buttons.pressed(index);
    if (pressed != raw_pressed_[index]) {
      raw_pressed_[index] = pressed;
      raw_changed_ms_[index] = now_ms;
    }

    if (pressed == stable_pressed_[index] || now_ms - raw_changed_ms_[index] < debounce_ms_) {
      continue;
    }

    stable_pressed_[index] = pressed;
    if (pressed) {
      event->index = index;
      return true;
    }
  }

  return false;
}

}  // namespace app
