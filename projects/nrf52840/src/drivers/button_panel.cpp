#include "drivers/button_panel.h"

namespace drivers {

ButtonPanel::ButtonPanel(const ButtonConfig* buttons, size_t count)
    : buttons_(buttons), count_(count) {}

void ButtonPanel::begin() const {
  for (size_t index = 0; index < count_; index++) {
    pinMode(buttons_[index].pin, INPUT_PULLUP);
  }
}

size_t ButtonPanel::count() const {
  return count_;
}

const ButtonConfig& ButtonPanel::at(size_t index) const {
  return buttons_[index];
}

bool ButtonPanel::pressed(size_t index) const {
  return digitalRead(buttons_[index].pin) == LOW;
}

}  // namespace drivers
