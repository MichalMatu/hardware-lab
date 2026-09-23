/**
 * @file Rf433PowerManager.h
 * @brief RF433 module power control
 * 
 * Manages power pin for RF433 transmitter module:
 * - On-demand power control (similar to sensor power management)
 * - Stabilization delays before/after transmission
 * - GPIO initialization
 */

#pragma once

#include <cstdint>

namespace Rf433PowerManager {

/**
 * @brief Initialize power control GPIO
 * 
 * Sets up the power pin as OUTPUT. Must be called once before use.
 */
void init();

/**
 * @brief Check if power manager is initialized
 */
bool isInitialized();

/**
 * @brief Turn on RF433 module power with stabilization delay
 * 
 * Powers on the module and waits for voltage stabilization.
 * Safe to call multiple times (idempotent).
 * 
 * @note Blocks for POWER_ON_STABILIZATION_MS (from config)
 */
void powerOn();

/**
 * @brief Turn off RF433 module power after transmission delay
 * 
 * Waits a short time to ensure transmission completed, then powers off.
 * Safe to call when already off.
 * 
 * @note Blocks for POWER_OFF_DELAY_MS (from config)
 */
void powerOff();

}  // namespace Rf433PowerManager
