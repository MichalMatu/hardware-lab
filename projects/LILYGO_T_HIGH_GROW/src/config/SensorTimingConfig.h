/**
 * @file SensorTimingConfig.h
 * @brief Sensor task configuration and timing parameters
 * 
 * Sensor read intervals, power management, I2C addresses, and task parameters.
 * Note: Sensor calibration is stored in SensorConfig.h/cpp (runtime preferences).
 */

#ifndef SensorTimingConfig_h
#define SensorTimingConfig_h

#include <Arduino.h>

// ============================================================================
// Sensor Task Configuration
// ============================================================================
namespace SENSOR {
    // Task parameters
    constexpr uint32_t STACK_SIZE = 4096;
    constexpr uint8_t TASK_PRIORITY = 1;
    constexpr uint8_t TASK_CORE = 1;
    
    // Timing intervals
    constexpr uint32_t READ_INTERVAL_MS = 5000;     // 5s sensor read
    constexpr uint32_t LOG_INTERVAL_MS = 300000;    // 5min log to CSV
    
    // Sensor-specific settings
    constexpr uint32_t POWER_STABILIZE_DELAY_MS = 1000;  // Wait after POWER_CTRL
    constexpr uint8_t SALT_SAMPLES = 120;  // Salt sensor averaging (vendor uses 120)
    constexpr uint8_t SALT_SAMPLE_DELAY_MS = 2;
    constexpr uint8_t INTER_SENSOR_DELAY_MS = 5;  // Delay between different sensors
    constexpr bool POWER_ON_DEMAND = true;        // Gate sensor power per read/log cycle
    
    // Sensor read retries and delays
    constexpr uint8_t MAX_READ_ATTEMPTS = 3;        // Retry failed sensor reads up to 3 times
    constexpr uint32_t READ_RETRY_DELAY_MS = 50;   // Wait 50ms between retry attempts
    constexpr uint32_t POWER_ON_STABILIZE_MS = 500; // DHT stabilization after power-on (POWER_ON_DEMAND)
    constexpr uint32_t BH1750_INIT_DELAY_MS = 180;  // BH1750 wake delay after power gate (120ms measurement + margin)
    
    // Task loop sleep interval
    constexpr uint32_t TASK_LOOP_SLEEP_MS = 500;    // Sleep between loop iterations (low power idle)
    
    // I2C sensor addresses
    constexpr uint8_t BH1750_ADDR = 0x23;
}

// ============================================================================
// Data Logger Configuration
// ============================================================================
namespace LOGGER {
    constexpr const char* BASE_PATH = "/data";
    constexpr const char* FILE_EXT = ".csv";
    constexpr uint32_t MAX_FILE_SIZE_BYTES = 1048576;  // 1MB
    constexpr uint8_t MAX_FILES_PER_DIR = 31;  // Max days in month
}

// ============================================================================
// Button Configuration
// ============================================================================
namespace BTN {
    // Hardware debounce time - ignores interrupts within this window
    constexpr uint32_t DEBOUNCE_MS = 300;
    
    // Duration user must hold button to trigger factory reset
    // Referenced in: ButtonTask.cpp, docs/power_modes.md mentions >20s but code uses 10s
    constexpr uint32_t LONG_PRESS_MS = 10000;
    
    // Log heartbeat interval while button is held (prevents log spam)
    // Used in: ButtonTask.cpp
    constexpr uint32_t HOLD_LOG_INTERVAL_MS = 2000;
}

#endif
