#include "PowerSleepController.h"
#include "PowerWakeController.h"
#include "../system/Logging.h"
#include "../config/AppConfig.h"
#include <core/Features.h>

#if FT_ENABLED(FT_SLEEP)
#include <services/SleepService.h>
#endif

namespace POWER {

bool PowerSleepController::_sleepRequested = false;
uint32_t PowerSleepController::_sleepRequestAtMs = 0;
uint32_t PowerSleepController::_sleepDelayMs = 0;
const char* PowerSleepController::_sleepReason = nullptr;
uint32_t PowerSleepController::_wakeIntervalMs = POWER::WAKE_INTERVAL_MS;
void (*PowerSleepController::_preSleepHook)() = nullptr;
void (*PowerSleepController::_sleepCallback)(const char *reason) = nullptr;
bool PowerSleepController::_isEnteringDeepSleep = false;

void PowerSleepController::begin() {
    _sleepRequested = false;
    _sleepRequestAtMs = 0;
    _sleepDelayMs = 0;
    _isEnteringDeepSleep = false;
    _wakeIntervalMs = POWER::WAKE_INTERVAL_MS;
}

void PowerSleepController::setWakeInterval(uint32_t intervalMs) {
    _wakeIntervalMs = intervalMs;
}

uint32_t PowerSleepController::getWakeInterval() {
    return _wakeIntervalMs;
}

void PowerSleepController::requestSleep(const char *reason, uint32_t delayMs) {
    if (delayMs == 0) {
        _sleepRequested = true;
        _sleepDelayMs = 0;
        _sleepRequestAtMs = millis();
        _sleepReason = reason;

        LOGI("[Power] Sleep requested (%s)", reason ? reason : "unknown");
        enterDeepSleep(reason);
        return;
    }

    _sleepRequested = true;
    _sleepDelayMs = delayMs;
    _sleepRequestAtMs = millis();
    _sleepReason = reason;
    LOGI("[Power] Sleep requested (%s, +%lu ms)", reason ? reason : "unknown", static_cast<unsigned long>(delayMs));
}

bool PowerSleepController::isSleepRequested() {
    return _sleepRequested;
}

uint32_t PowerSleepController::getSleepEtaMs() {
    if (!_sleepRequested) {
        return 0;
    }
    uint32_t now = millis();
    uint32_t elapsed = now - _sleepRequestAtMs;
    if (elapsed >= _sleepDelayMs) {
        return 0;
    }
    return _sleepDelayMs - elapsed;
}

void PowerSleepController::loopTick() {
    if (_sleepRequested) {
        // Immediate sleep request (delay=0): sleep entry is already handled in requestSleep for synchronous,
        // but if delay > 0 we handle it here.
        if (_sleepDelayMs > 0) {
            uint32_t now = millis();
            if (now - _sleepRequestAtMs >= _sleepDelayMs) {
                enterDeepSleep(_sleepReason ? _sleepReason : "pending");
            }
        }
    }
}

void PowerSleepController::enterDeepSleep(const char *reason) {
    if (_sleepCallback) {
        _sleepCallback(reason);
        return;
    }

    if (_isEnteringDeepSleep) {
        LOGW("[Power] enterDeepSleep() called twice; ignoring (reason=%s)", reason ? reason : "unknown");
        return;
    }
    _isEnteringDeepSleep = true;
    _sleepRequested = true;
    _sleepDelayMs = 0;

    LOGI("[Power] Entering deep sleep (reason: %s). Wake timer: %lus, button: GPIO%d LOW.",
         reason ? reason : "unknown",
         static_cast<unsigned long>(_wakeIntervalMs / 1000UL),
         HW::USER_BUTTON);
    
    // Call framework sleep callbacks
#if FT_ENABLED(FT_SLEEP)
    SleepService::executeSleepCallbacks();
#endif
    
    // Call application pre-sleep hook
    if (_preSleepHook) {
        _preSleepHook();
    }
    
    PowerWakeController::configureWakeSources(_wakeIntervalMs);
    LOGI("[Power] Calling esp_deep_sleep_start() now");
    delay(20); // give UART a moment to flush
    esp_deep_sleep_start();
}

void PowerSleepController::setPreSleepHook(void (*hook)()) {
    _preSleepHook = hook;
}

void PowerSleepController::setSleepCallback(void (*callback)(const char *)) {
    _sleepCallback = callback;
}

void PowerSleepController::resetTestHooks() {
    _sleepCallback = nullptr;
}

} // namespace POWER
