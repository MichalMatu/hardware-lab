#include "PowerConfig.h"
#include "../system/Logging.h"

namespace POWER {

Preferences PowerConfig::_prefs;
InactivityConfig PowerConfig::_cfg{0, 0};

namespace {
    constexpr const char *kNamespace = "power_cfg";
    constexpr const char *kKeyInact = "inact_ms";
    constexpr const char *kKeyGrace = "grace_ms";
}

void PowerConfig::begin() {
    if (!_prefs.begin(kNamespace, false)) {
        LOGE("prefs begin failed, using defaults");
    }
    _cfg.timeoutMs = _prefs.getUInt(kKeyInact, POWER::INACTIVITY_TIMEOUT_MS);
    _cfg.graceAfterBootMs = _prefs.getUInt(kKeyGrace, POWER::GRACE_AFTER_BOOT_MS);
}

void PowerConfig::setInactivityTimeout(uint32_t timeoutMs) {
    _cfg.timeoutMs = timeoutMs;
    _prefs.putUInt(kKeyInact, timeoutMs);
    LOGI("Saved inactivity=%u ms", timeoutMs);
}

void PowerConfig::setGracePeriod(uint32_t graceMs) {
    _cfg.graceAfterBootMs = graceMs;
    _prefs.putUInt(kKeyGrace, graceMs);
    LOGI("Saved grace=%u ms", graceMs);
}

uint32_t PowerConfig::getInactivityTimeout() {
    return _cfg.timeoutMs;
}

uint32_t PowerConfig::getGracePeriod() {
    return _cfg.graceAfterBootMs;
}

InactivityConfig PowerConfig::getInactivityConfig() {
    return _cfg;
}

} // namespace POWER
