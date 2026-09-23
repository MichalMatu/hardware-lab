#include "PowerManager.h"
#include "../system/Logging.h"
#include "../config/AppConfig.h"
#include <WiFi.h>
#include <Preferences.h>
#include <driver/gpio.h>
#include <core/Features.h>
#if FT_ENABLED(FT_SLEEP)
#include <services/SleepService.h>
#endif

#undef LOG_TAG
#define LOG_TAG "Power"

namespace POWER {







uint32_t PowerManager::nowMs() {
    return PowerActivityTracker::nowMs();
}

void PowerManager::begin() {
    PowerConfig::begin();
    
    // Cache values for local use just for this log, or use getters directly
    uint32_t timeout = PowerConfig::getInactivityTimeout();
    uint32_t grace = PowerConfig::getGracePeriod();
    
    PowerSleepController::begin();
    PowerWakeController::begin();
    PowerActivityTracker::begin();

    WakeReason wakeReason = PowerWakeController::getWakeReason();
    LOGI("[Power] Inactivity=%lu ms, grace=%lu ms, wake=%lu ms, reason=%d", static_cast<unsigned long>(timeout), static_cast<unsigned long>(grace), static_cast<unsigned long>(PowerSleepController::getWakeInterval()), static_cast<int>(wakeReason));
}

void PowerManager::setInactivityTimeout(uint32_t timeoutMs) {
    PowerConfig::setInactivityTimeout(timeoutMs);
}

void PowerManager::setGracePeriod(uint32_t graceMs) {
    PowerConfig::setGracePeriod(graceMs);
}

uint32_t PowerManager::getInactivityTimeout() {
    return PowerConfig::getInactivityTimeout();
}

uint32_t PowerManager::getGracePeriod() {
    return PowerConfig::getGracePeriod();
}

void PowerManager::notifyActivity(const char *source) {
    PowerActivityTracker::notifyActivity(source);
}

void PowerManager::loopTick() {
    PowerSleepController::loopTick();

    if (PowerSleepController::isSleepRequested()) {
        return;
    }
    
    PowerActivityTracker::loopTick();
}

void PowerManager::requestSleep(const char *reason, uint32_t delayMs) {
    PowerSleepController::requestSleep(reason, delayMs);
}

bool PowerManager::isSleepRequested() {
    return PowerSleepController::isSleepRequested();
}

WakeReason PowerManager::wakeReason() {
    return PowerWakeController::getWakeReason();
}

InactivityConfig PowerManager::inactivityConfig() {
    return PowerConfig::getInactivityConfig();
}

uint32_t PowerManager::lastActivityMs() {
    return PowerActivityTracker::getLastActivityMs();
}

uint32_t PowerManager::bootMs() {
    return PowerActivityTracker::getBootMs();
}

uint32_t PowerManager::wakeIntervalMs() {
    return PowerSleepController::getWakeInterval();
}

uint32_t PowerManager::sleepEtaMs() {
    return PowerSleepController::getSleepEtaMs();
}

void PowerManager::setPreSleepHook(void (*hook)()) {
    PowerSleepController::setPreSleepHook(hook);
}

void PowerManager::setTimeProvider(uint32_t (*provider)()) {
    PowerActivityTracker::setTimeProvider(provider);
}

void PowerManager::setApStationsProvider(int (*provider)()) {
    PowerActivityTracker::setApStationsProvider(provider);
}

void PowerManager::setSleepCallback(void (*callback)(const char *)) {
    PowerSleepController::setSleepCallback(callback);
}

void PowerManager::setConfigureWakeSourcesCallback(void (*callback)()) {
    PowerWakeController::setConfigureWakeSourcesCallback(callback);
}

void PowerManager::resetTestHooks() {
    PowerActivityTracker::resetTestHooks();
    PowerWakeController::resetTestHooks();
    PowerSleepController::resetTestHooks();
}

}  // namespace POWER
