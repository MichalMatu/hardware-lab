/**
 * @file SystemConfig.h
 * @brief Application and system-level configuration
 * 
 * Application metadata, main loop timing, and factory reset settings.
 */

#ifndef SystemConfig_h
#define SystemConfig_h

#include <Arduino.h>

// ============================================================================
// Application & System
// ============================================================================
namespace APP {
    constexpr const char* VERSION = "2.0.0";
    constexpr const char* NAME = "PlantStatus";
    constexpr const char* DEVICE = "LILYGO-T-HIGROW";
    
    // Main application loop delay
    // Used in: Application.cpp - loop()
    // Prevents tight loop, allows background tasks to run
    constexpr uint32_t MAIN_LOOP_DELAY_MS = 100;
}

// ============================================================================
// Factory Reset
// ============================================================================
namespace FACTORY {
    // Log heartbeat interval during factory reset operations
    // Used in: FactoryReset.cpp - performFactoryReset()
    // Prevents watchdog timeout by showing progress during LittleFS format
    // Format can take several seconds; periodic logging proves system is alive
    constexpr uint32_t PROGRESS_LOG_INTERVAL_MS = 2000;
    
    // Delay before restart after factory reset
    // Used in: FactoryReset.cpp
    // Gives time for final logs to be output before system restart
    constexpr uint32_t PRE_RESTART_DELAY_MS = 500;
}

#endif
