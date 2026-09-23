#include "Logger.h"

#include <stdarg.h>
#include <stdio.h>

#include <esp_heap_caps.h>

Logger Log;

void Logger::begin() {
  Serial.begin(115200);
  delay(50);
}

void Logger::setLevel(LogLevel level) {
  level_ = level;
}

void Logger::log(LogLevel level, const char* tag, const char* message) {
  if (static_cast<uint8_t>(level) < static_cast<uint8_t>(level_)) {
    return;
  }
  Serial.printf("[%10lu] [%s] %s\n", millis(), tag, message);
}

void Logger::logf(LogLevel level, const char* tag, const char* fmt, ...) {
  if (static_cast<uint8_t>(level) < static_cast<uint8_t>(level_)) {
    return;
  }

  char buffer[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  log(level, tag, buffer);
}

void Logger::heap(const char* tag) {
  logf(LogLevel::Debug, tag, "heap=%u minHeap=%u psram=%u",
       ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getFreePsram());
}
