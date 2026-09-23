/**
 * @file PowerConfig.h
 * @brief Power management and deep sleep configuration
 * 
 * Inactivity timeouts, grace periods, wake intervals, and validation ranges.
 */

#ifndef PowerConfig_h
#define PowerConfig_h

#include <Arduino.h>

// ============================================================================
// Power Management
// ============================================================================
namespace POWER {
    // === Default Runtime Behavior ===
    
    // Inactivity timeout in always_on mode
    // Used in: PowerManager.cpp (default)
    // After this period without HTTP requests or button presses, system enters deep sleep
    // Can be overridden at runtime via PowerManager preferences or /rest/power/config API
    constexpr uint32_t INACTIVITY_TIMEOUT_MS = 120000;   // 2min default
    
    // Grace period after boot before inactivity timer activates
    // Used in: PowerManager.cpp (default)
    // Allows user to interact with device/UI after boot without immediate sleep
    constexpr uint32_t GRACE_AFTER_BOOT_MS = 30000;      // 30s default
    
    // Deep sleep timer wake interval
    // Used in: PowerManager.cpp - esp_sleep_enable_timer_wakeup()
    // Device wakes after this period even if button is not pressed
    constexpr uint32_t WAKE_INTERVAL_MS = 600000;        // 10 minutes
    
    // === API Validation Ranges ===
    
    // Minimum allowed inactivity timeout via API
    // Used in: PowerApiService.cpp - POST /rest/power/mode validation
    // Prevents accidental near-instant sleep (5s minimum for reasonable interaction)
    constexpr uint32_t INACTIVITY_TIMEOUT_MIN_MS = 5000;      // 5s
    
    // Maximum allowed inactivity timeout via API
    // Used in: PowerApiService.cpp - POST /rest/power/mode validation
    // Caps timeout to 24h (prevents overflow, ensures periodic wake for health checks)
    constexpr uint32_t INACTIVITY_TIMEOUT_MAX_MS = 86400000;  // 24h
    
    // Maximum allowed grace period via API
    // Used in: PowerApiService.cpp - POST /rest/power/mode validation
    // Caps grace to 10min (balances UX vs battery life; prevents indefinite grace)
    constexpr uint32_t GRACE_MAX_MS = 600000;                 // 10min
    
    // === Sleep Transition & Logging ===
    
    // Interval for logging inactivity countdown
    // Used in: PowerManager.cpp - loopTick()
    // Prints countdown updates to help user track time until sleep
    constexpr uint32_t COUNTDOWN_LOG_INTERVAL_MS = 10000;    // 10s

}  // namespace POWER

#endif
