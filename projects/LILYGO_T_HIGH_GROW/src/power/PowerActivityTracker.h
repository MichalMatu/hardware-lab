#pragma once

#include <Arduino.h>

namespace POWER {

class PowerActivityTracker {
public:
    static void begin();
    static void loopTick();
    static void notifyActivity(const char *source = nullptr);
    
    static uint32_t getLastActivityMs();
    static uint32_t getBootMs();
    static uint32_t nowMs();

    // Test hooks
    static void setTimeProvider(uint32_t (*provider)());
    static void setApStationsProvider(int (*provider)());
    static void resetTestHooks();

private:
    static uint32_t _bootMs;
    static uint32_t _lastActivityMs;
    
    // Logging state
    static uint32_t _lastCountdownLog;
    static bool _apClientLogged;
    static bool _loggedGrace;
    static uint32_t _lastActivityLogMs;
    static String _lastActivitySource;

    static uint32_t (*_timeProvider)();
    static int (*_apStationsProvider)();
};

} // namespace POWER
