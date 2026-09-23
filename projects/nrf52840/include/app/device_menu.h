#pragma once

#include <Arduino.h>
#include <stddef.h>

namespace app {

/// OLED menu state machine for selecting a future BLE HID device mode.
class DeviceMenu {
 public:
  DeviceMenu();

  /// Semantic button indexes consumed by the menu.
  enum ButtonIndex : size_t {
    /// Move to the previous visible item.
    kButtonUp = 0,
    /// Move to the next visible item.
    kButtonDown = 1,
    /// Enter a submenu or run the selected action placeholder.
    kButtonSelect = 2,
    /// Return from an action list to the device list.
    kButtonBack = 3,
  };

  /// Resets the menu to the device list.
  void begin();
  /// Applies one debounced button press to the menu state.
  void handle_button(size_t button_index);

  /// Current screen title shown on the OLED.
  const char* title() const;
  /// Current short status line shown below the title.
  const char* subtitle() const;
  /// Number of items on the current screen.
  size_t item_count() const;
  /// Display text for an item on the current screen.
  const char* item(size_t index) const;
  /// Currently selected item index on the current screen.
  size_t selected_index() const;
  /// True when the caller should redraw the OLED menu.
  bool dirty() const;
  /// Marks the current menu state as rendered.
  void clear_dirty();

 private:
  enum class Screen : uint8_t {
    kDeviceList,
    kActionList,
  };

  void move_previous();
  void move_next();
  void select();
  void back();
  size_t current_count() const;
  const char* device_name(size_t index) const;
  const char* action_name(size_t device_index, size_t action_index) const;
  void log_placeholder_action() const;

  Screen screen_;
  size_t selected_device_;
  size_t selected_action_;
  const char* last_action_;
  bool dirty_;
};

}  // namespace app
