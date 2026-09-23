#ifndef HARDWARE_RTC_MODULE_H
#define HARDWARE_RTC_MODULE_H

/**
 * @brief RTC DS3231 module with power management for battery-operated mode
 * 
 * Features:
 * - Dynamic power control on GPIO 12 (ON only during reads, ~150ms)
 * - Secondary I2C bus (Wire1) on GPIO 13/15 to avoid conflicts
 * - Thread-safe operations with FreeRTOS mutex
 * - Time synchronization between RTC ↔ ESP32 system time ↔ NTP
 * 
 * Hardware connections:
 * - RTC_POWER (GPIO 12): Power supply control
 * - RTC_SDA (GPIO 13): I2C data line
 * - RTC_SCL (GPIO 15): I2C clock line
 * - RTC battery: CR2032 for backup (maintains time during deep sleep)
 * 
 * Module structure (all files in src/hardware/rtc/):
 * - RTCCommon.h/cpp:  Shared state and helper functions
 * - RTCPower.h/cpp:   Power management (on/off control)
 * - RTCCore.h/cpp:    Initialization and basic read/write
 * - RTCSync.h/cpp:    Time synchronization (RTC ↔ system time)
 * - RTCStatus.h/cpp:  HTTP API endpoint for status
 */

#include "RTCCommon.h"
#include "RTCPower.h"
#include "RTCCore.h"
#include "RTCSync.h"
#include "RTCStatus.h"

// Public API namespace (backward compatibility)
namespace RTCModule {
    // Core operations
    using Core::begin;
    using Core::readAndPrint;
    using Core::setTime;
    
    // Synchronization
    using Sync::syncWithSystemTime;
    using Sync::syncSystemTimeFromRTC;
    using Sync::restoreSystemTimeOrFallback;
    using Sync::getLastSyncTime;
    using Sync::updateLastSyncTime;
    
    // HTTP API
    using Status::getStatus;
}

#endif
