/**
 * @file Rf433Config.h
 * @brief Configuration structures and constants for RF433 integration
 * 
 * Defines device model, transmission settings, and system limits.
 * Uses plain DRAM (no PSRAM) with compile-time bounds.
 */

#ifndef Rf433Config_h
#define Rf433Config_h

#include <Arduino.h>
#include <vector>
#include <cstdint>

// ============================================================================
// System Limits (DRAM constraints)
// ============================================================================
namespace RF433 {
    constexpr size_t MAX_DEVICES = 32;           // Maximum stored devices
    constexpr size_t MAX_LABEL_LENGTH = 32;      // Device label max chars
    constexpr size_t MAX_ID_LENGTH = 24;         // Device ID max chars
    
    // Transmission defaults
    constexpr uint8_t DEFAULT_PROTOCOL = 1;      // RCSwitch protocol 1
    constexpr uint8_t DEFAULT_BIT_LENGTH = 24;   // Standard 24-bit codes
    constexpr uint16_t DEFAULT_PULSE_LENGTH = 0; // 0 = use protocol default
    constexpr uint8_t DEFAULT_REPEAT_COUNT = 10; // Standard repeat
    constexpr uint8_t MIN_REPEAT_COUNT = 1;
    constexpr uint8_t MAX_REPEAT_COUNT = 50;
    
    // Power management
    constexpr uint32_t POWER_ON_STABILIZATION_MS = 50;   // Delay after power on
    constexpr uint32_t POWER_OFF_DELAY_MS = 100;         // Delay before power off
    
    // FreeRTOS task config
    constexpr size_t TX_QUEUE_SIZE = 8;          // Max queued transmissions
    constexpr uint32_t TX_TASK_STACK_SIZE = 3072; // Stack for TX task
    constexpr UBaseType_t TX_TASK_PRIORITY = 1;  // Low priority task
}

// ============================================================================
// Device Definition
// ============================================================================
struct Rf433Device {
    String id;              // Unique identifier (e.g. "lamp_living_room")
    String label;           // Human-readable name
    uint32_t codeOn;        // RF code for ON command
    uint32_t codeOff;       // RF code for OFF command
    uint8_t bitLength;      // Number of bits (typically 24 or 32)
    uint8_t protocol;       // RCSwitch protocol number (1-12)
    uint16_t pulseLength;   // Pulse length in µs (0 = use protocol default)
    uint8_t repeat;         // Repeat count for transmission
    
    Rf433Device() 
        : codeOn(0), codeOff(0), 
          bitLength(RF433::DEFAULT_BIT_LENGTH),
          protocol(RF433::DEFAULT_PROTOCOL),
          pulseLength(RF433::DEFAULT_PULSE_LENGTH),
          repeat(RF433::DEFAULT_REPEAT_COUNT) {}
    
    bool isValid() const {
        return !id.isEmpty() && 
               id.length() <= RF433::MAX_ID_LENGTH &&
               !label.isEmpty() &&
               label.length() <= RF433::MAX_LABEL_LENGTH &&
               (codeOn > 0 || codeOff > 0) &&
               bitLength > 0 && bitLength <= 32 &&
               protocol >= 1 && protocol <= 12 &&
               repeat >= RF433::MIN_REPEAT_COUNT && 
               repeat <= RF433::MAX_REPEAT_COUNT;
    }
};

// ============================================================================
// Device State (runtime tracking)
// ============================================================================
struct Rf433DeviceState {
    String deviceId;
    bool lastCommandOn;     // true = last sent ON, false = last sent OFF
    uint32_t lastTransmitMs; // millis() of last transmission
    
    Rf433DeviceState() : lastCommandOn(false), lastTransmitMs(0) {}
    Rf433DeviceState(const String& id, bool on) 
        : deviceId(id), lastCommandOn(on), lastTransmitMs(millis()) {}
};

// ============================================================================
// Global Settings
// ============================================================================
struct Rf433Settings {
    bool enabled;           // Master enable/disable
    uint8_t powerPin;       // GPIO for power control
    uint8_t txPin;          // GPIO for data transmission
    
    Rf433Settings()
        : enabled(true),
          powerPin(23),     // Default from HardwareConfig
          txPin(17) {}
};

#endif // Rf433Config_h
