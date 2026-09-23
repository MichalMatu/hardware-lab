#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "../config/AppConfig.h"

namespace POWER {

struct InactivityConfig {
    uint32_t timeoutMs;
    uint32_t graceAfterBootMs;
};

class PowerConfig {
public:
    static void begin();
    static void setInactivityTimeout(uint32_t timeoutMs);
    static void setGracePeriod(uint32_t graceMs);
    static uint32_t getInactivityTimeout();
    static uint32_t getGracePeriod();
    static InactivityConfig getInactivityConfig();

private:
    static Preferences _prefs;
    static InactivityConfig _cfg;
};

} // namespace POWER
