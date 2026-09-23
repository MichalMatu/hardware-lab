#include "app/status_led.h"

namespace app {

StatusLed::StatusLed(uint8_t pin, uint8_t on_state, uint32_t interval_ms)
    : pin_(pin),
      on_state_(on_state),
      interval_ms_(interval_ms),
      last_toggle_ms_(0),
      enabled_(false) {}

void StatusLed::begin() {
  pinMode(pin_, OUTPUT);
  set(false);
}

bool StatusLed::update(uint32_t now_ms) {
  if (now_ms - last_toggle_ms_ < interval_ms_) {
    return false;
  }

  last_toggle_ms_ = now_ms;
  set(!enabled_);
  return true;
}

bool StatusLed::enabled() const {
  return enabled_;
}

void StatusLed::set(bool enabled) {
  enabled_ = enabled;
  digitalWrite(pin_, enabled_ ? on_state_ : off_state());
}

uint8_t StatusLed::off_state() const {
  return on_state_ == HIGH ? LOW : HIGH;
}

}  // namespace app
