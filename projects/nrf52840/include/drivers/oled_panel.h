#pragma once

#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

#include "drivers/button_panel.h"

namespace drivers {

/// SSD1306 OLED driver with startup I2C checks and menu rendering helpers.
class OledPanel {
 public:
  /// Creates a display driver for one I2C OLED wiring configuration.
  OledPanel(uint8_t power_pin, bool uses_gpio_power, uint8_t sda_pin, uint8_t scl_pin,
            uint16_t width, uint16_t height, uint32_t i2c_clock_hz, uint32_t update_interval_ms);

  /// Initializes power handling, I2C, and the SSD1306 controller.
  void begin(const ButtonPanel& buttons);
  /// Redraws the legacy button status screen when its interval elapsed.
  void update(uint32_t now_ms, const ButtonPanel& buttons);
  /// Renders the current application menu state.
  void render_menu(const char* title, const char* subtitle, size_t selected_index,
                   size_t item_count, const char* (*item_at)(void* context, size_t index),
                   void* context);
  /// True when the SSD1306 controller initialized successfully.
  bool ready() const;
  /// Detected I2C address, or zero before successful initialization.
  uint8_t address() const;

 private:
  void begin_power();
  void ensure_wire();
  bool i2c_lines_idle() const;
  void print_i2c_line_state() const;
  void recover_i2c_bus();
  bool begin_display_at(uint8_t address);
  void force_display_on();
  void draw_boot_screen();
  void draw_status_screen(const ButtonPanel& buttons);

  uint8_t power_pin_;
  bool uses_gpio_power_;
  uint8_t sda_pin_;
  uint8_t scl_pin_;
  uint32_t i2c_clock_hz_;
  uint32_t update_interval_ms_;
  uint32_t last_update_ms_;
  Adafruit_SSD1306 display_;
  bool ready_;
  bool wire_ready_;
  uint8_t address_;
};

}  // namespace drivers
