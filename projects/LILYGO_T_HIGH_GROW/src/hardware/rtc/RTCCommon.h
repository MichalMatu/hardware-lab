#ifndef HARDWARE_RTC_COMMON_H
#define HARDWARE_RTC_COMMON_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace RTCModule {
    namespace Internal {
        // Shared RTC hardware instance
        extern RTC_DS3231 rtc;
        
        // Initialization flag
        extern bool initialized;
        
        // Thread-safe mutex
        extern SemaphoreHandle_t rtcMutex;
        
        // Last successful sync timestamp
        extern time_t lastSyncTime;
        
        // Validation constants
        constexpr int kMinValidYear = 2025;
        constexpr int kMaxValidYear = 2040;
        constexpr time_t kFallbackUnixUtc = 1735689600;  // 2025-01-01 00:00:00 UTC
        
        // Helper functions
        bool isYearValid(int year);
        bool lockRtc(TickType_t timeoutTicks = pdMS_TO_TICKS(250));
        void unlockRtc();
    }
}

#endif
