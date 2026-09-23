#include "PowerActivityTracker.h"
#include "PowerConfig.h"
#include "PowerSleepController.h"
#include "../system/Logging.h"
#include "../config/AppConfig.h"
#include <WiFi.h>

namespace POWER {

uint32_t PowerActivityTracker::_bootMs = 0;
uint32_t PowerActivityTracker::_lastActivityMs = 0;
uint32_t PowerActivityTracker::_lastCountdownLog = 0;
bool PowerActivityTracker::_apClientLogged = false;
bool PowerActivityTracker::_loggedGrace = false;
uint32_t PowerActivityTracker::_lastActivityLogMs = 0;
String PowerActivityTracker::_lastActivitySource;
uint32_t (*PowerActivityTracker::_timeProvider)() = nullptr;
int (*PowerActivityTracker::_apStationsProvider)() = nullptr;

uint32_t PowerActivityTracker::nowMs() {
    if (_timeProvider) {
        return _timeProvider();
    }
    return millis();
}

void PowerActivityTracker::begin() {
    _bootMs = nowMs();
    _lastActivityMs = _bootMs;
    _lastCountdownLog = 0;
    _apClientLogged = false;
    _loggedGrace = false;
    _lastActivityLogMs = 0;
    _lastActivitySource = "";
}

uint32_t PowerActivityTracker::getLastActivityMs() {
    return _lastActivityMs;
}

uint32_t PowerActivityTracker::getBootMs() {
    return _bootMs;
}

void PowerActivityTracker::notifyActivity(const char *source) {
    _lastActivityMs = nowMs();
    if (!source || !*source) {
        return;
    }

    // Avoid self-spam: the status UI polls api/rest endpoints (tail, config).
    // Still track activity for sleep logic, but keep logs quiet by default.
    const bool noisyHttpSource = (strncmp(source, "api/", 4) == 0) || (strncmp(source, "rest/", 5) == 0);
    const uint32_t minLogIntervalMs = noisyHttpSource ? 60000UL : 10000UL;

    uint32_t now = nowMs();
    const bool sourceChanged = (_lastActivitySource != source);
    const bool allowSourceChangedLog = !noisyHttpSource;

    if ((allowSourceChangedLog && sourceChanged) || (now - _lastActivityLogMs >= minLogIntervalMs)) {
        if (noisyHttpSource) {
            LOGD("[Power] Activity: %s", source);
        } else {
            LOGI("[Power] Activity: %s", source);
        }
        _lastActivityLogMs = now;
        _lastActivitySource = source;
    }
}

void PowerActivityTracker::loopTick() {
    // If sleep is already requested, we don't need to check for inactivity
    if (PowerSleepController::isSleepRequested()) {
        return;
    }

    uint32_t now = nowMs();

    // Treat AP station presence as activity to avoid sleeping while user connects
    int apStations = _apStationsProvider ? _apStationsProvider() : WiFi.softAPgetStationNum();
    if (apStations > 0) {
        if (!_apClientLogged) {
            LOGI("[Power] AP client detected (%d). Resetting inactivity timer.", apStations);
            _apClientLogged = true;
        }
        notifyActivity("ap-station");
        _lastCountdownLog = now;  // reset countdown logging window
        return;                  // do not sleep while AP clients are connected
    } else {
        _apClientLogged = false;
    }

    uint32_t gracePeriod = PowerConfig::getGracePeriod();
    if (now - _bootMs < gracePeriod) {
        if (!_loggedGrace) {
            LOGI("[Power] Grace period active (%lu ms left)", static_cast<unsigned long>(gracePeriod - (now - _bootMs)));
            _loggedGrace = true;
        }
        return;
    }

    uint32_t timeoutMs = PowerConfig::getInactivityTimeout();
    if (timeoutMs == 0) {
        return;
    }
    
    uint32_t idleMs = now - _lastActivityMs;
    if (idleMs >= timeoutMs) {
        // Single-line, user-friendly (avoid separate *details* log).
        LOGI("[Power] Inactivity timeout -> sleep (idle=%lus tmo=%lus)",
             static_cast<unsigned long>(idleMs / 1000UL),
             static_cast<unsigned long>(timeoutMs / 1000UL));
        PowerSleepController::requestSleep("inactivity");
        return;
    }
    
    // Periodic countdown log every 30s (defined in AppConfig as POWER::COUNTDOWN_LOG_INTERVAL_MS)
    if (now - _lastCountdownLog >= POWER::COUNTDOWN_LOG_INTERVAL_MS) {
        uint32_t remainingMs = (idleMs >= timeoutMs) ? 0 : (timeoutMs - idleMs);
        uint32_t remainingSec = remainingMs / 1000UL;
        // Single-line, compressed (avoid separate *details* log).
        LOGI("[Power] Sleep in %lus (idle=%lus/%lus)",
             static_cast<unsigned long>(remainingSec),
             static_cast<unsigned long>(idleMs / 1000UL),
             static_cast<unsigned long>(timeoutMs / 1000UL));
        _lastCountdownLog = now;
    }
}

void PowerActivityTracker::setTimeProvider(uint32_t (*provider)()) {
    _timeProvider = provider;
}

void PowerActivityTracker::setApStationsProvider(int (*provider)()) {
    _apStationsProvider = provider;
}

void PowerActivityTracker::resetTestHooks() {
    _timeProvider = nullptr;
    _apStationsProvider = nullptr;
}

} // namespace POWER
