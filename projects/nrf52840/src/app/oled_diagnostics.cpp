#include "app/oled_diagnostics.h"

namespace app {

OledDiagnostics::OledDiagnostics(uint8_t power_pin, bool uses_gpio_power, uint8_t sda_pin,
                                 uint8_t scl_pin, uint32_t i2c_clock_hz,
                                 uint32_t heartbeat_interval_ms)
    : power_pin_(power_pin),
      uses_gpio_power_(uses_gpio_power),
      sda_pin_(sda_pin),
      scl_pin_(scl_pin),
      i2c_clock_hz_(i2c_clock_hz),
      heartbeat_interval_ms_(heartbeat_interval_ms),
      last_heartbeat_ms_(0),
      power_enabled_(!uses_gpio_power),
      wire_ready_(false) {}

void OledDiagnostics::begin() {
  Serial.println("OLED diagnostic mode: on");
  Serial.print("OLED power source: ");
  Serial.println(uses_gpio_power_ ? "GPIO" : "external VCC");
  Serial.print("OLED SCL:   PIN_");
  Serial.println(scl_pin_);
  Serial.print("OLED SDA:   PIN_");
  Serial.println(sda_pin_);

  if (uses_gpio_power_) {
    disable_power();
  } else {
    pinMode(power_pin_, INPUT);
    Serial.println("OLED GPIO power disabled; expecting external VCC");
  }

  print_help();
}

void OledDiagnostics::update(uint32_t now_ms, const drivers::ButtonPanel& buttons) {
  while (Serial.available() > 0) {
    handle_command(static_cast<char>(Serial.read()), buttons);
  }

  if (now_ms - last_heartbeat_ms_ < heartbeat_interval_ms_) {
    return;
  }

  last_heartbeat_ms_ = now_ms;
  Serial.print("OLED diag heartbeat power=");
  Serial.println(power_enabled_ ? "on" : "off");
}

void OledDiagnostics::handle_command(char command, const drivers::ButtonPanel& buttons) {
  switch (command) {
    case 'p':
    case 'P':
      if (uses_gpio_power_) {
        soft_start_power();
      } else {
        Serial.println("OLED GPIO power disabled; OLED should be powered from VCC");
        power_enabled_ = true;
      }
      break;
    case 'o':
    case 'O':
      if (uses_gpio_power_) {
        disable_power();
      } else {
        pinMode(power_pin_, INPUT);
        Serial.println("OLED GPIO power disabled; PIN_017 left high-Z");
        power_enabled_ = true;
      }
      break;
    case 's':
    case 'S':
      if (!power_enabled_) {
        Serial.println("I2C scan skipped: OLED power is off, send 'p' first");
        break;
      }
      scan_i2c();
      break;
    case 'l':
    case 'L':
      print_i2c_line_state();
      break;
    case 'r':
    case 'R':
      recover_i2c_bus();
      break;
    case 'b':
    case 'B':
      report_buttons(buttons);
      break;
    case 'h':
    case 'H':
    case '?':
      print_help();
      break;
    case '\n':
    case '\r':
    case ' ':
      break;
    default:
      Serial.println("Unknown OLED diag command, send '?' for help");
      break;
  }
}

void OledDiagnostics::print_help() const {
  Serial.println("OLED diagnostic commands:");
  Serial.println("  p - soft-start OLED power when GPIO power is enabled");
  Serial.println("  o - set OLED power pin high-Z/off when GPIO power is enabled");
  Serial.println("  l - read SDA/SCL levels without I2C transfer");
  Serial.println("  s - scan I2C once");
  Serial.println("  r - try I2C bus recovery pulses");
  Serial.println("  b - print button states");
  Serial.println("  ? - print this help");
}

void OledDiagnostics::ensure_wire() {
  if (wire_ready_) {
    return;
  }

  Wire.setPins(sda_pin_, scl_pin_);
  Wire.begin();
  Wire.setClock(i2c_clock_hz_);
  wire_ready_ = true;
}

bool OledDiagnostics::i2c_lines_idle() const {
  pinMode(sda_pin_, INPUT_PULLUP);
  pinMode(scl_pin_, INPUT_PULLUP);
  delay(5);
  return digitalRead(sda_pin_) == HIGH && digitalRead(scl_pin_) == HIGH;
}

void OledDiagnostics::print_i2c_line_state() const {
  pinMode(sda_pin_, INPUT_PULLUP);
  pinMode(scl_pin_, INPUT_PULLUP);
  delay(5);

  Serial.print("I2C lines: SDA=");
  Serial.print(digitalRead(sda_pin_) == HIGH ? "HIGH" : "LOW");
  Serial.print(" SCL=");
  Serial.println(digitalRead(scl_pin_) == HIGH ? "HIGH" : "LOW");
}

void OledDiagnostics::recover_i2c_bus() {
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
  wire_ready_ = false;
  print_i2c_line_state();
}

uint8_t OledDiagnostics::scan_i2c() {
  uint8_t found = 0;

  print_i2c_line_state();
  if (!i2c_lines_idle()) {
    Serial.println("I2C scan skipped: bus is not idle");
    return 0;
  }

  ensure_wire();

  Serial.println("I2C scan:");
  for (uint8_t candidate_address = 1; candidate_address < 127; candidate_address++) {
    Wire.beginTransmission(candidate_address);
    const uint8_t error = Wire.endTransmission();
    if (error == 0) {
      found++;
      Serial.print("  0x");
      if (candidate_address < 16) {
        Serial.print("0");
      }
      Serial.println(candidate_address, HEX);
    }
  }

  if (found == 0) {
    Serial.println("  no devices found");
  }

  return found;
}

void OledDiagnostics::disable_power() {
  digitalWrite(power_pin_, LOW);
  pinMode(power_pin_, INPUT);
  power_enabled_ = false;
  Serial.println("OLED power pin high-Z/off");
}

void OledDiagnostics::soft_start_power() {
  digitalWrite(power_pin_, LOW);
  pinMode(power_pin_, INPUT);
  Serial.println("OLED power pin high-Z");
  delay(1000);

  Serial.println("OLED power soft-start pulses");
  for (uint8_t duty = 1; duty <= 8; duty++) {
    const uint8_t on_ms = duty * 2;
    const uint8_t off_ms = (9 - duty) * 2;

    for (uint8_t pulse = 0; pulse < 4; pulse++) {
      digitalWrite(power_pin_, HIGH);
      pinMode(power_pin_, OUTPUT);
      delay(on_ms);
      pinMode(power_pin_, INPUT);
      delay(off_ms);
    }
  }

  digitalWrite(power_pin_, HIGH);
  pinMode(power_pin_, OUTPUT);
  delay(500);
  power_enabled_ = true;
  Serial.println("OLED power pin HIGH");
}

void OledDiagnostics::report_buttons(const drivers::ButtonPanel& buttons) const {
  Serial.print("Buttons:");
  for (size_t index = 0; index < buttons.count(); index++) {
    Serial.print(" ");
    Serial.print(buttons.at(index).name);
    Serial.print("=");
    Serial.print(buttons.pressed(index) ? "DOWN" : "up");
  }
  Serial.println();
}

}  // namespace app
