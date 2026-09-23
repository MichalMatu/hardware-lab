/**
 * @file RTCConfig.h
 * @brief RTC and time synchronization configuration
 * 
 * DS3231 RTC module timing, NTP sync parameters, and mutex timeouts.
 */

#ifndef RTCConfig_h
#define RTCConfig_h

#include <Arduino.h>

// ============================================================================
// RTC & Time Synchronization
// ============================================================================
namespace RTC {
    // Mutex timeout when initializing DS3231 module
    // Used in: RTCModule.cpp - lockRtc() during begin()
    // Lower timeout acceptable during init (expect quick success/fail)
    constexpr uint32_t INIT_MUTEX_TIMEOUT_MS = 1000;
    
    // Mutex timeout for RTC write operations (setTime)
    // Used in: RTCModule.cpp - lockRtc() during setTime()
    // Longer timeout for I2C write operations which may be slower
    constexpr uint32_t WRITE_MUTEX_TIMEOUT_MS = 2000;
    
    // Throttle interval between NTP->RTC synchronizations
    // Used in: RTCTask.cpp
    // Prevents excessive RTC writes which could wear flash or cause I2C congestion
    constexpr uint32_t MIN_SYNC_INTERVAL_MS = 60000;
    
    
    // DS3231 hardware timing
    constexpr uint32_t POWER_STABILIZE_DELAY_MS = 500;  // Wait after powering RTC on (extra margin for I2C stability)
    constexpr uint32_t I2C_OPERATION_DELAY_MS = 10;     // Short delay between I2C operations
    
    // SNTP sync detection (fallback when SNTP_SYNC_STATUS unavailable)
    constexpr int32_t TIME_JUMP_THRESHOLD_SEC = 5;      // Detect time jump >5s as new sync
    constexpr time_t SYNC_VALID_AFTER_FALLBACK_SEC = 86400; // 24h after fallback = valid time
    // Grace period after boot before attempting first NTP sync
    // Used in: RTCTask.cpp
    // Allows network/WiFi to stabilize before time sync attempts
    constexpr uint32_t FIRST_SYNC_DELAY_MS = 30000;
    
    // Main RTCTask sleep interval between sync checks
    // Used in: RTCTask.cpp - vTaskDelay()
    // Keep CPU idle most of the time; NTP syncs are infrequent
    constexpr uint32_t TASK_SLEEP_MS = 60000;
}

#endif
