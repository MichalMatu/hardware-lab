/**
 * @file NetworkConfig.h
 * @brief Network and API timeouts and connection parameters
 * 
 * WiFi connection timeouts, HTTP API timeouts, and filesystem mutex settings.
 */

#ifndef NetworkConfig_h
#define NetworkConfig_h

#include <Arduino.h>

// ============================================================================
// Network & Connection
// ============================================================================
namespace NET {
    constexpr uint32_t CONNECT_TIMEOUT_MS = 10000;  // 10s WiFi connection timeout
}

// ============================================================================
// HTTP & API Timeouts
// ============================================================================
namespace API {
    // Filesystem mutex timeout for LittleFS operations
    // Used in: LogsApiService.cpp, ChartsApiService.cpp, SensorLoggingTask.cpp
    // Guards concurrent access to filesystem during CSV read/write/delete
    // HTTP handlers will return 503 if mutex can't be acquired within this time
    constexpr uint32_t FS_MUTEX_TIMEOUT_MS = 2000;
    
    // Maximum wait time for fresh sensor data when forceRead is requested
    // Used in: SensorsApiService.cpp - /rest/sensors/current endpoint
    // Polls snapshot every 50ms until fresh data arrives or timeout
    constexpr uint32_t SENSOR_DATA_WAIT_MS = 2000;
    
    // Maximum age of cached sensor data before triggering fresh read
    // Used in: SensorsApiService.cpp - /rest/sensors/current endpoint
    // If snapshot is older than this, automatic refresh is triggered
    // Balance: lower = fresher data but more sensor reads; higher = stale but efficient
    constexpr uint32_t SENSOR_MAX_AGE_MS = 30000;
}

#endif
