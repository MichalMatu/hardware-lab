#include "Logging.h"

#include <stdarg.h>

namespace LOG {

namespace {
constexpr uint16_t kFixedRingSize = 20;

const char *levelLabel(esp_log_level_t level) {
    switch (level) {
        case ESP_LOG_ERROR: return "E";
        case ESP_LOG_WARN: return "W";
        case ESP_LOG_INFO: return "I";
        case ESP_LOG_DEBUG: return "D";
        case ESP_LOG_VERBOSE: return "V";
        case ESP_LOG_NONE:
        default: return "N";
    }
}

esp_log_level_t normalizeLevel(esp_log_level_t level) {
    if (level < ESP_LOG_NONE) return ESP_LOG_NONE;
    if (level > ESP_LOG_VERBOSE) return ESP_LOG_VERBOSE;
    return level;
}

}  // namespace

Settings Logging::_settings{ESP_LOG_INFO, kFixedRingSize};

void Logging::begin(const Settings &settings) {
    setSettings(settings);
    RingBuffer::begin(kFixedRingSize);
    // Align ESP-IDF global log level with configured level so we see early boot logs
    esp_log_level_set("*", _settings.level);
}

void Logging::setSettings(const Settings &settings) {
    _settings.level = normalizeLevel(settings.level);
    (void)settings.ringBufferSize;
    _settings.ringBufferSize = kFixedRingSize;
    RingBuffer::resize(kFixedRingSize);
    // Apply immediately (not only on boot)
    esp_log_level_set("*", _settings.level);
}

Settings Logging::settings() {
    return _settings;
}

bool Logging::isEnabled(esp_log_level_t level) {
    return level <= _settings.level;
}

bool Logging::shouldPersist(esp_log_level_t level) {
    return level <= ESP_LOG_ERROR;
}

void Logging::log(esp_log_level_t level, const char *tag, const char *fmt, ...) {
    if (!isEnabled(level)) {
        return;
    }

    char message[192];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    // Ensure newline-delimited output on UART
    esp_log_write(level, tag, "%s\n", message);

    if (_settings.ringBufferSize > 0) {
        RingBuffer::append(levelLabel(level), tag, message);
    }
}

std::vector<Line> Logging::tail(size_t maxLines) {
    return RingBuffer::tail(maxLines);
}

void Logging::clearBuffer() {
    RingBuffer::clear();
}

const char *Logging::levelToString(esp_log_level_t level) {
    switch (normalizeLevel(level)) {
        case ESP_LOG_ERROR: return "error";
        case ESP_LOG_WARN: return "warn";
        case ESP_LOG_INFO: return "info";
        case ESP_LOG_DEBUG: return "debug";
        case ESP_LOG_VERBOSE: return "verbose";
        case ESP_LOG_NONE:
        default: return "none";
    }
}

esp_log_level_t Logging::stringToLevel(const String &name, esp_log_level_t fallback) {
    String lower = name;
    lower.toLowerCase();
    if (lower == "error") return ESP_LOG_ERROR;
    if (lower == "warn" || lower == "warning") return ESP_LOG_WARN;
    if (lower == "info") return ESP_LOG_INFO;
    if (lower == "debug") return ESP_LOG_DEBUG;
    if (lower == "verbose") return ESP_LOG_VERBOSE;
    if (lower == "none") return ESP_LOG_NONE;
    return fallback;
}

}  // namespace LOG
