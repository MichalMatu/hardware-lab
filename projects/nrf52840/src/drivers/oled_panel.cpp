#include "drivers/oled_panel.h"

namespace drivers {
namespace {
constexpr uint8_t kPrimaryOledAddress = 0x3C;
constexpr uint8_t kSecondaryOledAddress = 0x3D;
}  // namespace

OledPanel::OledPanel(uint8_t power_pin, bool uses_gpio_power, uint8_t sda_pin, uint8_t scl_pin,
                     uint16_t width, uint16_t height, uint32_t i2c_clock_hz,
                     uint32_t update_interval_ms)
    : power_pin_(power_pin),
      uses_gpio_power_(uses_gpio_power),
      sda_pin_(sda_pin),
      scl_pin_(scl_pin),
      i2c_clock_hz_(i2c_clock_hz),
      update_interval_ms_(update_interval_ms),
      last_update_ms_(0),
      display_(width, height, &Wire, -1),
      ready_(false),
      wire_ready_(false),
      address_(0) {}

void OledPanel::begin(const ButtonPanel& buttons) {
  Serial.print("OLED power source: ");
  Serial.println(uses_gpio_power_ ? "GPIO" : "external VCC");
  Serial.print("OLED SCL:   PIN_");
  Serial.println(scl_pin_);
  Serial.print("OLED SDA:   PIN_");
  Serial.println(sda_pin_);

  begin_power();
  print_i2c_line_state();
  if (!i2c_lines_idle()) {
    recover_i2c_bus();
  }

  if (!i2c_lines_idle()) {
    Serial.println("OLED init skipped: I2C bus is not idle");
    return;
  }

  ensure_wire();

  if (!begin_display_at(kPrimaryOledAddress) && !begin_display_at(kSecondaryOledAddress)) {
    Serial.println("OLED init failed at 0x3C and 0x3D");
    return;
  }

  Serial.print("OLED initialized at 0x");
  Serial.println(address_, HEX);
  draw_status_screen(buttons);
}

void OledPanel::update(uint32_t now_ms, const ButtonPanel& buttons) {
  if (!ready_ || now_ms - last_update_ms_ < update_interval_ms_) {
    return;
  }

  last_update_ms_ = now_ms;
  draw_status_screen(buttons);
}

void OledPanel::render_menu(const char* title, const char* subtitle, size_t selected_index,
                            size_t item_count, const char* (*item_at)(void* context, size_t index),
                            void* context) {
  if (!ready_) {
    return;
  }

  constexpr size_t kVisibleItems = 5;
  size_t first_item = 0;
  if (selected_index >= kVisibleItems) {
    first_item = selected_index - kVisibleItems + 1;
  }

  display_.clearDisplay();
  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  display_.setCursor(0, 0);
  display_.println(title);

  display_.setCursor(0, 10);
  display_.println(subtitle);

  for (size_t row = 0; row < kVisibleItems; row++) {
    const size_t item_index = first_item + row;
    if (item_index >= item_count) {
      break;
    }

    display_.setCursor(0, 22 + row * 8);
    display_.print(item_index == selected_index ? ">" : " ");
    display_.println(item_at(context, item_index));
  }

  display_.display();
}

bool OledPanel::ready() const {
  return ready_;
}

uint8_t OledPanel::address() const {
  return address_;
}

void OledPanel::begin_power() {
  if (uses_gpio_power_) {
    digitalWrite(power_pin_, HIGH);
    pinMode(power_pin_, OUTPUT);
    delay(250);
    return;
  }

  pinMode(power_pin_, INPUT);
  Serial.println("OLED GPIO power disabled; using external VCC");
}

void OledPanel::ensure_wire() {
  if (wire_ready_) {
    return;
  }

  Wire.setPins(sda_pin_, scl_pin_);
  Wire.begin();
  Wire.setClock(i2c_clock_hz_);
  wire_ready_ = true;
}

bool OledPanel::i2c_lines_idle() const {
  pinMode(sda_pin_, INPUT_PULLUP);
  pinMode(scl_pin_, INPUT_PULLUP);
  delay(5);
  return digitalRead(sda_pin_) == HIGH && digitalRead(scl_pin_) == HIGH;
}

void OledPanel::print_i2c_line_state() const {
  pinMode(sda_pin_, INPUT_PULLUP);
  pinMode(scl_pin_, INPUT_PULLUP);
  delay(5);

  Serial.print("I2C lines: SDA=");
  Serial.print(digitalRead(sda_pin_) == HIGH ? "HIGH" : "LOW");
  Serial.print(" SCL=");
  Serial.println(digitalRead(scl_pin_) == HIGH ? "HIGH" : "LOW");
}

void OledPanel::recover_i2c_bus() {
  Serial.println("I2C recovery: toggling SCL");
  pinMode(sda_pin_, INPUT_PULLUP);
  pinMode(scl_pin_, OUTPUT);

  for (uint8_t pulse = 0; pulse < 9; pulse++) {
    digitalWrite(scl_pin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(scl_pin_, LOW);
    delayMicroseconds(10);
  }

  pinMode(scl_pin_, INPUT_PULLUP);
  delay(5);
  print_i2c_line_state();
}

bool OledPanel::begin_display_at(uint8_t address) {
  if (!display_.begin(SSD1306_SWITCHCAPVCC, address)) {
    return false;
  }

  address_ = address;
  ready_ = true;
  force_display_on();
  draw_boot_screen();
  return true;
}

void OledPanel::force_display_on() {
  display_.ssd1306_command(SSD1306_DISPLAYON);
  display_.ssd1306_command(SSD1306_SETCONTRAST);
  display_.ssd1306_command(0xFF);
  display_.dim(false);
  display_.invertDisplay(false);

  display_.clearDisplay();
  display_.fillScreen(SSD1306_WHITE);
  display_.display();
  delay(500);
  display_.clearDisplay();
  display_.display();
}

void OledPanel::draw_boot_screen() {
  display_.clearDisplay();
  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  display_.setCursor(0, 0);
  display_.println(APP_NAME);
  display_.print("OLED 0x");
  display_.println(address_, HEX);
  display_.println("I2C: SDA 022 SCL 020");
  display_.println(uses_gpio_power_ ? "Power: GPIO" : "Power: VCC");
  display_.display();
}

void OledPanel::draw_status_screen(const ButtonPanel& buttons) {
  display_.clearDisplay();
  display_.setCursor(0, 0);
  display_.println("nRF52840 OLED");
  display_.print("addr: 0x");
  display_.println(address_, HEX);
  display_.print("uptime: ");
  display_.print(millis() / 1000);
  display_.println("s");
  display_.println();

  for (size_t index = 0; index < buttons.count(); index++) {
    display_.print(buttons.at(index).name);
    display_.print(": ");
    display_.println(buttons.pressed(index) ? "DOWN" : "up");
  }

  display_.display();
}

}  // namespace drivers
