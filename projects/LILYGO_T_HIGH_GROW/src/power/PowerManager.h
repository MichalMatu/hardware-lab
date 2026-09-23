#pragma once

#include <Arduino.h>
#include <esp_sleep.h>
#include "../config/AppConfig.h"
#include "PowerConfig.h"
#include "PowerWakeController.h"
#include "PowerSleepController.h"
#include "PowerActivityTracker.h"

namespace POWER {

class PowerManager {
public:
    static void begin();
    static void setInactivityTimeout(uint32_t timeoutMs);
    static void setGracePeriod(uint32_t graceMs);
    static uint32_t getInactivityTimeout();
    static uint32_t getGracePeriod();
    static void notifyActivity(const char *source = nullptr);
    static void loopTick();
    static void requestSleep(const char *reason, uint32_t delayMs = 0);
    static bool isSleepRequested();
    static WakeReason wakeReason();
    static InactivityConfig inactivityConfig();
    static uint32_t lastActivityMs();
    static uint32_t bootMs();
    static uint32_t wakeIntervalMs();
    static uint32_t sleepEtaMs();

    static void setPreSleepHook(void (*hook)());

    // Test hooks (no effect in production if unset)
    static void setTimeProvider(uint32_t (*provider)());
    static void setApStationsProvider(int (*provider)());
    static void setSleepCallback(void (*callback)(const char*));
    static void setConfigureWakeSourcesCallback(void (*callback)());
    static void resetTestHooks();

private:
    static uint32_t nowMs();
};

}  // namespace POWER
