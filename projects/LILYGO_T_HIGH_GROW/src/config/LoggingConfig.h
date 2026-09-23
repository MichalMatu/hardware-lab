#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <esp_log.h>
#include "../system/Logging.h"

namespace LoggingConfig {

LOG::Settings get();
void begin();
void save(const LOG::Settings &settings);
void setLevel(esp_log_level_t level);
void setRingBufferSize(uint16_t size);

}  // namespace LoggingConfig
