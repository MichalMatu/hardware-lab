/**
 * @file Rf433RadioController.h
 * @brief Asynchronous RF433 transmitter controller with FreeRTOS task
 * 
 * Non-blocking RF433 transmission using dedicated FreeRTOS task and queue.
 * Adapted from vendor/rf433/core/Rf433RadioController with power pin control.
 */

#pragma once

#include <cstdint>

namespace Rf433RadioController {

/**
 * @brief Transmission configuration per frame
 */
struct TxConfig {
    uint8_t protocol;       // RCSwitch protocol (1-12)
    uint8_t repeat;         // Repeat count
    uint16_t pulseLength;   // Pulse length in µs (0 = use protocol default)
};

/**
 * @brief Initialize the RF433 transmitter task.
 * 
 * Creates a dedicated FreeRTOS task for non-blocking RF transmission.
 * Must be called once during system startup before any sendFrame() calls.
 * Sets up GPIO for power control and TX pin.
 */
void init();

/**
 * @brief Check if the RF433 task is running and ready.
 * 
 * @return true if initialized and ready to accept transmissions
 */
bool isReady();

/**
 * @brief Queue an RF frame for asynchronous transmission.
 * 
 * This is non-blocking - returns immediately after queuing.
 * The actual transmission happens in a dedicated FreeRTOS task.
 * Power pin is automatically managed (on before TX, off after with delay).
 * 
 * @param value The code to transmit
 * @param bitLength Number of bits (typically 24 or 32)
 * @param config Protocol, repeat count, and pulse length
 * @param logLabel Human-readable label for logs (optional)
 * @return true if queued successfully, false if queue full or not initialized
 */
bool sendFrame(uint32_t value,
               uint8_t bitLength,
               const TxConfig& config,
               const char* logLabel = nullptr);

/**
 * @brief Synchronous send - blocks until transmission complete.
 * 
 * Use only when you need confirmation that transmission finished.
 * Timeout prevents indefinite blocking.
 * 
 * @param timeoutMs Maximum time to wait for transmission (default 2000ms)
 * @return true if transmitted, false on timeout or error
 */
bool sendFrameSync(uint32_t value,
                   uint8_t bitLength,
                   const TxConfig& config,
                   const char* logLabel = nullptr,
                   uint32_t timeoutMs = 2000);

}  // namespace Rf433RadioController
