#pragma once

#include <Arduino.h>
#include "AppTypes.h"

class Logger {
 public:
  void begin();
  void setLevel(LogLevel level);
  void log(LogLevel level, const char* tag, const char* message);
  void logf(LogLevel level, const char* tag, const char* fmt, ...);
  void heap(const char* tag);

 private:
  LogLevel level_ = LogLevel::Info;
};

extern Logger Log;

#define LOGD(tag, msg) Log.log(LogLevel::Debug, tag, msg)
#define LOGI(tag, msg) Log.log(LogLevel::Info, tag, msg)
#define LOGW(tag, msg) Log.log(LogLevel::Warn, tag, msg)
#define LOGE(tag, msg) Log.log(LogLevel::Error, tag, msg)
