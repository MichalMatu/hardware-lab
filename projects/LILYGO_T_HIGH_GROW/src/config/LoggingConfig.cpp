#include "LoggingConfig.h"

#include <Preferences.h>

namespace {
constexpr const char *kNamespace = "log_cfg";
constexpr const char *kKeyLevel = "lvl";
Preferences prefs;
bool initialized = false;
LOG::Settings cached{ESP_LOG_INFO, 20};

void ensureInit() {
    if (initialized) {
        return;
    }
    if (!prefs.begin(kNamespace, false)) {
        return;
    }
    // Single schema: one log level.
    cached.level = static_cast<esp_log_level_t>(prefs.getChar(kKeyLevel, static_cast<char>(cached.level)));
    // Ring buffer size is fixed (ESP32 DRAM/fragmentation constraints)
    cached.ringBufferSize = 20;
    initialized = true;
}

void store() {
    if (!initialized) {
        ensureInit();
    }
    // Persist only the unified level going forward.
    prefs.putChar(kKeyLevel, static_cast<char>(cached.level));
}

}  // namespace

namespace LoggingConfig {

LOG::Settings get() {
    ensureInit();
    return cached;
}

void begin() {
    ensureInit();
}

void save(const LOG::Settings &settings) {
    ensureInit();
    cached.level = settings.level;
    cached.ringBufferSize = 20;
    store();
}

void setLevel(esp_log_level_t level) {
    ensureInit();
    cached.level = level;
    store();
}

void setRingBufferSize(uint16_t size) {
    ensureInit();
    (void)size;
    cached.ringBufferSize = 20;
    store();
}

}  // namespace LoggingConfig
