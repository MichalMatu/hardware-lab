/**
 * @file HardwareConfig.h
 * @brief Hardware pin definitions and I2C addresses
 * 
 * All GPIO pin assignments and hardware-specific constants.
 * LILYGO TTGO-T-HIGrow specific pinout.
 */

#ifndef HardwareConfig_h
#define HardwareConfig_h

#include <Arduino.h>

// ============================================================================
// Hardware Pin Definitions
// ============================================================================
namespace HW {
    constexpr uint8_t I2C_SDA = 25;
    constexpr uint8_t I2C_SCL = 26;
    constexpr uint8_t DHT_PIN = 16;
    constexpr uint8_t SOIL_PIN = 32;
    constexpr uint8_t SALT_PIN = 34;
    constexpr uint8_t BAT_ADC = 33;
    constexpr uint8_t POWER_CTRL = 4;
    constexpr uint8_t USER_BUTTON = 35;
    constexpr uint8_t LED_PIN = 2;  // Moved from GPIO 13 to avoid RTC conflict
    
    // RTC DS3231 pins
    constexpr uint8_t RTC_POWER = 12;  // Power control for RTC module
    constexpr uint8_t RTC_SDA = 13;    // I2C1 data line
    constexpr uint8_t RTC_SCL = 15;    // I2C1 clock line
    
    // RF433 transmitter pins
    constexpr uint8_t RF433_POWER = 23;  // Power control for RF433 module
    constexpr uint8_t RF433_TX = 17;     // Data pin for transmission
}

// ============================================================================
// Serial & Communication
// ============================================================================
namespace COM {
    constexpr uint32_t SERIAL_BAUD_RATE = 115200;
}

#endif
