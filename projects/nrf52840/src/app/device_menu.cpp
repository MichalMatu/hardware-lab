#include "app/device_menu.h"

namespace app {
namespace {
constexpr const char* kDevices[] = {
    "Keyboard", "Mouse", "Media", "Gamepad", "Diagnostics",
};

constexpr const char* kKeyboardActions[] = {
    "Copy",
    "Paste",
    "Enter",
    "Escape",
};

constexpr const char* kMouseActions[] = {
    "Left click",
    "Right click",
    "Scroll up",
    "Scroll down",
};

constexpr const char* kMediaActions[] = {
    "Play/Pause",
    "Volume +",
    "Volume -",
    "Next track",
};

constexpr const char* kGamepadActions[] = {
    "Button A",
    "Button B",
    "D-pad up",
    "D-pad down",
};

constexpr const char* kDiagnosticActions[] = {
    "I2C levels",
    "I2C scan",
    "Button test",
    "BLE status",
};

constexpr size_t kDeviceCount = sizeof(kDevices) / sizeof(kDevices[0]);
constexpr size_t kKeyboardActionCount = sizeof(kKeyboardActions) / sizeof(kKeyboardActions[0]);
constexpr size_t kMouseActionCount = sizeof(kMouseActions) / sizeof(kMouseActions[0]);
constexpr size_t kMediaActionCount = sizeof(kMediaActions) / sizeof(kMediaActions[0]);
constexpr size_t kGamepadActionCount = sizeof(kGamepadActions) / sizeof(kGamepadActions[0]);
constexpr size_t kDiagnosticActionCount =
    sizeof(kDiagnosticActions) / sizeof(kDiagnosticActions[0]);

const char* const* actions_for_device(size_t device_index) {
  switch (device_index) {
    case 0:
      return kKeyboardActions;
    case 1:
      return kMouseActions;
    case 2:
      return kMediaActions;
    case 3:
      return kGamepadActions;
    default:
      return kDiagnosticActions;
  }
}

size_t action_count_for_device(size_t device_index) {
  switch (device_index) {
    case 0:
      return kKeyboardActionCount;
    case 1:
      return kMouseActionCount;
    case 2:
      return kMediaActionCount;
    case 3:
      return kGamepadActionCount;
    default:
      return kDiagnosticActionCount;
  }
}
}  // namespace

DeviceMenu::DeviceMenu()
    : screen_(Screen::kDeviceList),
      selected_device_(0),
      selected_action_(0),
      last_action_("Idle"),
      dirty_(true) {}

void DeviceMenu::begin() {
  screen_ = Screen::kDeviceList;
  selected_device_ = 0;
  selected_action_ = 0;
  last_action_ = "Idle";
  dirty_ = true;
}

void DeviceMenu::handle_button(size_t button_index) {
  switch (button_index) {
    case kButtonUp:
      move_previous();
      break;
    case kButtonDown:
      move_next();
      break;
    case kButtonSelect:
      select();
      break;
    case kButtonBack:
      back();
      break;
    default:
      break;
  }
}

const char* DeviceMenu::title() const {
  return screen_ == Screen::kDeviceList ? "Device type" : device_name(selected_device_);
}

const char* DeviceMenu::subtitle() const {
  return last_action_;
}

size_t DeviceMenu::item_count() const {
  return current_count();
}

const char* DeviceMenu::item(size_t index) const {
  if (screen_ == Screen::kDeviceList) {
    return device_name(index);
  }

  return action_name(selected_device_, index);
}

size_t DeviceMenu::selected_index() const {
  return screen_ == Screen::kDeviceList ? selected_device_ : selected_action_;
}

bool DeviceMenu::dirty() const {
  return dirty_;
}

void DeviceMenu::clear_dirty() {
  dirty_ = false;
}

void DeviceMenu::move_previous() {
  const size_t count = current_count();
  if (count == 0) {
    return;
  }

  size_t* selected = screen_ == Screen::kDeviceList ? &selected_device_ : &selected_action_;
  *selected = *selected == 0 ? count - 1 : *selected - 1;
  dirty_ = true;
}

void DeviceMenu::move_next() {
  const size_t count = current_count();
  if (count == 0) {
    return;
  }

  size_t* selected = screen_ == Screen::kDeviceList ? &selected_device_ : &selected_action_;
  *selected = (*selected + 1) % count;
  dirty_ = true;
}

void DeviceMenu::select() {
  if (screen_ == Screen::kDeviceList) {
    screen_ = Screen::kActionList;
    selected_action_ = 0;
    last_action_ = device_name(selected_device_);
    dirty_ = true;
    return;
  }

  log_placeholder_action();
  last_action_ = action_name(selected_device_, selected_action_);
  dirty_ = true;
}

void DeviceMenu::back() {
  if (screen_ == Screen::kDeviceList) {
    last_action_ = "Idle";
    dirty_ = true;
    return;
  }

  screen_ = Screen::kDeviceList;
  last_action_ = "Select type";
  dirty_ = true;
}

size_t DeviceMenu::current_count() const {
  return screen_ == Screen::kDeviceList ? kDeviceCount : action_count_for_device(selected_device_);
}

const char* DeviceMenu::device_name(size_t index) const {
  return index < kDeviceCount ? kDevices[index] : "";
}

const char* DeviceMenu::action_name(size_t device_index, size_t action_index) const {
  const size_t count = action_count_for_device(device_index);
  if (action_index >= count) {
    return "";
  }

  return actions_for_device(device_index)[action_index];
}

void DeviceMenu::log_placeholder_action() const {
  Serial.print("HID placeholder: ");
  Serial.print(device_name(selected_device_));
  Serial.print(" -> ");
  Serial.println(action_name(selected_device_, selected_action_));
}

}  // namespace app
