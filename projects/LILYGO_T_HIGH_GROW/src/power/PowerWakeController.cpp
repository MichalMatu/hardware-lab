#include "PowerWakeController.h"
#include "../system/Logging.h"
#include "../config/AppConfig.h"
#include <driver/gpio.h>

namespace POWER {

WakeReason PowerWakeController::_wakeReason = WakeReason::Unknown;
void (*PowerWakeController::_configureWakeSourcesCallback)() = nullptr;

namespace {
    /**
     * @brief Check if GPIO pin maintains expected level over multiple samples
     */
    bool isPinStable(gpio_num_t pin, int expectedLevel, 
                     uint8_t samples = 7, 
                     uint8_t requiredMatches = 6, 
                     uint32_t sampleDelayMs = 2) {
        uint8_t matches = 0;
        for (uint8_t i = 0; i < samples; ++i) {
            int level = gpio_get_level(pin);
            if (level == expectedLevel) {
                ++matches;
            }
            if (sampleDelayMs > 0) {
                delay(sampleDelayMs);
            }
        }
        return matches >= requiredMatches;
    }
}

void PowerWakeController::begin() {
    auto cause = esp_sleep_get_wakeup_cause();
    switch (cause) {
        case ESP_SLEEP_WAKEUP_TIMER:
            _wakeReason = WakeReason::Timer;
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
        case ESP_SLEEP_WAKEUP_EXT1:
            _wakeReason = WakeReason::Button;
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            _wakeReason = WakeReason::Unknown;
            break;
        default:
            _wakeReason = WakeReason::Other;
            break;
    }
}

WakeReason PowerWakeController::getWakeReason() {
    return _wakeReason;
}

void PowerWakeController::configureWakeSources(uint32_t wakeIntervalMs) {
    if (_configureWakeSourcesCallback) {
        _configureWakeSourcesCallback();
        return;
    }

    // Timer wake
    esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(wakeIntervalMs) * 1000ULL);
    LOGI("[Power] Wake source armed: TIMER (%lus)", static_cast<unsigned long>(wakeIntervalMs / 1000UL));

    // Button wake (EXT0): active LOW on USER_BUTTON.
    const gpio_num_t buttonGpio = static_cast<gpio_num_t>(HW::USER_BUTTON);
    const int currentLevel = gpio_get_level(buttonGpio);
    const bool pinHigh = isPinStable(buttonGpio, 1);
    if (pinHigh) {
        esp_sleep_enable_ext0_wakeup(buttonGpio, 0);
        LOGI("[Power] Wake source armed: EXT0 (GPIO%d, wake on LOW). Level at sleep entry=%d", HW::USER_BUTTON, currentLevel);
    } else {
        LOGW("[Power] Button wake not armed (GPIO%d not stably HIGH at sleep entry; level=%d); using timer wake only", HW::USER_BUTTON, currentLevel);
    }
}

void PowerWakeController::setConfigureWakeSourcesCallback(void (*callback)()) {
    _configureWakeSourcesCallback = callback;
}

void PowerWakeController::resetTestHooks() {
    _configureWakeSourcesCallback = nullptr;
}

} // namespace POWER
