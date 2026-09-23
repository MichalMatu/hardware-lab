#pragma once

#include <Arduino.h>
#include <esp_sleep.h>

namespace POWER {

enum class WakeReason {
    Unknown,
    Timer,
    Button,
    Other
};

class PowerWakeController {
public:
    static void begin();
    static WakeReason getWakeReason();
    static void configureWakeSources(uint32_t wakeIntervalMs);
    
    // Test hooks
    static void setConfigureWakeSourcesCallback(void (*callback)());
    static void resetTestHooks();

private:
    static WakeReason _wakeReason;
    static void (*_configureWakeSourcesCallback)();
};

} // namespace POWER
