/**
 * @file Rf433InternalTypes.h
 * @brief Internal types and constants shared across RF433 modules
 * 
 * Contains structures and constants used internally by RF433 subsystem.
 * Not part of the public API exposed to application code.
 */

#pragma once

#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace Rf433Internal {

/**
 * @brief Maximum length for human-readable transmission labels in logs
 */
constexpr size_t kLogLabelMaxLen = 48;

/**
 * @brief Internal transmission request structure for FreeRTOS queue
 * 
 * Used to pass transmission requests from API layer to TX task.
 */
struct TxRequest {
    uint32_t value;                             // RF code to transmit
    uint8_t bitLength;                          // Number of bits
    uint8_t protocol;                           // RCSwitch protocol (1-12)
    uint8_t repeat;                             // Repeat count
    uint16_t pulseLength;                       // Pulse length in µs (0 = protocol default)
    SemaphoreHandle_t completionSemaphore;      // Optional: for sync send
    char logLabel[kLogLabelMaxLen];             // Human-readable label for logs
};

}  // namespace Rf433Internal
