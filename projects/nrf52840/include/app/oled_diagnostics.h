#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "drivers/button_panel.h"

namespace app {

/// Serial-controlled OLED and I2C recovery tool for hardware bring-up.
class OledDiagnostics {
 public:
  /// Creates diagnostics for one I2C OLED wiring configuration.
  OledDiagnostics(uint8_t power_pin, bool uses_gpio_power, uint8_t sda_pin, uint8_t scl_pin,
                  uint32_t i2c_clock_hz, uint32_t heartbeat_interval_ms);

  /// Prints command help and prepares GPIO power handling.
  void begin();
  /// Handles serial commands and emits periodic diagnostic heartbeats.
  void update(uint32_t now_ms, const drivers::ButtonPanel& buttons);

 private:
  void handle_command(char command, const drivers::ButtonPanel& buttons);
  void print_help() const;
  void ensure_wire();
  bool i2c_lines_idle() const;
  void print_i2c_line_state() const;
  void recover_i2c_bus();
  uint8_t scan_i2c();
  void disable_power();
  void soft_start_power();
  void report_buttons(const drivers::ButtonPanel& buttons) const;

  uint8_t power_pin_;
  bool uses_gpio_power_;
  uint8_t sda_pin_;
  uint8_t scl_pin_;
  uint32_t i2c_clock_hz_;
  uint32_t heartbeat_interval_ms_;
  uint32_t last_heartbeat_ms_;
  bool power_enabled_;
  bool wire_ready_;
};

}  // namespace app
