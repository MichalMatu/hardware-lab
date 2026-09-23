#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

#include "app/button_input.h"
#include "app/device_menu.h"
#include "app/oled_diagnostics.h"
#include "app/status_led.h"
#include "app_config.h"
#include "drivers/button_panel.h"
#include "drivers/oled_panel.h"

namespace {
constexpr drivers::ButtonConfig kButtons[] = {
    {"UP", app_config::kButtonK1Pin},
    {"DOWN", app_config::kButtonK2Pin},
    {"ENTER", app_config::kButtonK3Pin},
    {"RETURN", app_config::kButtonK4Pin},
};

drivers::ButtonPanel button_panel(kButtons, sizeof(kButtons) / sizeof(kButtons[0]));
app::ButtonInput button_input(app_config::kButtonDebounceMs);
app::DeviceMenu device_menu;
app::StatusLed status_led(app_config::kStatusLedPin, app_config::kStatusLedOnState,
                          app_config::kStatusBlinkIntervalMs);
app::OledDiagnostics oled_diagnostics(app_config::kOledPowerPin, app_config::kOledUsesGpioPower,
                                      app_config::kOledSdaPin, app_config::kOledSclPin,
                                      app_config::kI2cClockHz, app_config::kOledScanIntervalMs);
drivers::OledPanel oled_panel(app_config::kOledPowerPin, app_config::kOledUsesGpioPower,
                              app_config::kOledSdaPin, app_config::kOledSclPin,
                              app_config::kOledWidth, app_config::kOledHeight,
                              app_config::kI2cClockHz, app_config::kOledUpdateIntervalMs);
uint32_t last_menu_render_ms = 0;
uint32_t last_health_log_ms = 0;
bool oled_fallback_diagnostics = false;

const char* menu_item_at(void* context, size_t index) {
  return static_cast<app::DeviceMenu*>(context)->item(index);
}

void render_menu(uint32_t now_ms) {
  oled_panel.render_menu(device_menu.title(), device_menu.subtitle(), device_menu.selected_index(),
                         device_menu.item_count(), menu_item_at, &device_menu);
  last_menu_render_ms = now_ms;
  device_menu.clear_dirty();
}

void log_button_event(const app::ButtonEvent& event) {
  if (event.index >= button_panel.count()) {
    return;
  }

  const drivers::ButtonConfig& button = button_panel.at(event.index);
  Serial.print("button: ");
  Serial.print(button.name);
  Serial.print(" pin=");
  Serial.println(button.pin);
}

void log_health(uint32_t now_ms) {
  constexpr uint32_t kHealthLogIntervalMs = 5000UL;
  if (now_ms - last_health_log_ms < kHealthLogIntervalMs) {
    return;
  }

  last_health_log_ms = now_ms;
  Serial.print("health: oled=");
  Serial.print(oled_panel.ready() ? "ready" : "not-ready");
  Serial.print(" addr=0x");
  Serial.println(oled_panel.address(), HEX);
}
}  // namespace

// cppcheck-suppress unusedFunction
void setup() {
  status_led.begin();
  button_panel.begin();

  Serial.begin(app_config::kSerialBaudrate);
  delay(250);

  Serial.println();
  Serial.println(APP_NAME " " APP_VERSION);
  Serial.println("Board: nRF52840 SuperMini / nice!nano");
  Serial.println("Status: booted");

  if (app_config::kOledDiagnosticMode) {
    oled_diagnostics.begin();
  } else {
    oled_panel.begin(button_panel);
    if (oled_panel.ready()) {
      button_input.begin(button_panel, millis());
      device_menu.begin();
      render_menu(millis());
    } else {
      oled_fallback_diagnostics = true;
      Serial.println("OLED fallback diagnostics enabled");
      oled_diagnostics.begin();
    }
  }
}

// cppcheck-suppress unusedFunction
void loop() {
  const uint32_t now = millis();

  if (status_led.update(now)) {
    Serial.println(status_led.enabled() ? "led:on" : "led:off");
  }

  if (app_config::kOledDiagnosticMode || oled_fallback_diagnostics) {
    oled_diagnostics.update(now, button_panel);
  } else {
    log_health(now);

    app::ButtonEvent event;
    if (button_input.poll(now, button_panel, &event)) {
      log_button_event(event);
      device_menu.handle_button(event.index);
    }

    if (device_menu.dirty() || now - last_menu_render_ms >= app_config::kMenuRefreshIntervalMs) {
      render_menu(now);
    }
  }
}
