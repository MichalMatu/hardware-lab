#pragma once

#include <Arduino.h>
#include <esp_log.h>
#include "LogRingBuffer.h"

namespace LOG {

struct Settings {
    esp_log_level_t level;
    uint16_t ringBufferSize;
};

class Logging {
public:
    static void begin(const Settings &settings);
    static void setSettings(const Settings &settings);
    static Settings settings();

    static bool isEnabled(esp_log_level_t level);
    static bool shouldPersist(esp_log_level_t level);

    static void log(esp_log_level_t level, const char *tag, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

    static std::vector<Line> tail(size_t maxLines);
    static void clearBuffer();

    static const char *levelToString(esp_log_level_t level);
    static esp_log_level_t stringToLevel(const String &name, esp_log_level_t fallback);

private:
    static Settings _settings;
};

}  // namespace LOG

#ifndef LOG_TAG
#define LOG_TAG "App"
#endif

#define LOGD(fmt, ...) LOG::Logging::log(ESP_LOG_DEBUG, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) LOG::Logging::log(ESP_LOG_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) LOG::Logging::log(ESP_LOG_WARN, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOG::Logging::log(ESP_LOG_ERROR, LOG_TAG, fmt, ##__VA_ARGS__)
