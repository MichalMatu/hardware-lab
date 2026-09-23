#pragma once

#include <Arduino.h>

namespace POWER {

class PowerSleepController {
public:
    static void begin();
    static void loopTick();
    
    static void requestSleep(const char *reason, uint32_t delayMs = 0);
    static bool isSleepRequested();
    static uint32_t getSleepEtaMs();
    
    static void setWakeInterval(uint32_t intervalMs);
    static uint32_t getWakeInterval();

    static void setPreSleepHook(void (*hook)());
    
    // Test hooks
    static void setSleepCallback(void (*callback)(const char*));
    static void resetTestHooks();

private:
    static void enterDeepSleep(const char *reason);

    static bool _sleepRequested;
    static uint32_t _sleepRequestAtMs;
    static uint32_t _sleepDelayMs;
    static const char *_sleepReason;
    static uint32_t _wakeIntervalMs;
    
    static void (*_preSleepHook)();
    static void (*_sleepCallback)(const char *reason);
    
    static bool _isEnteringDeepSleep;
};

} // namespace POWER
