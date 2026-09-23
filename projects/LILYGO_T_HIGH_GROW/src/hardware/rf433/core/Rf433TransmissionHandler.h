/**
 * @file Rf433TransmissionHandler.h
 * @brief RF433 transmission logic using RCSwitch
 * 
 * Encapsulates RCSwitch radio control and actual transmission logic.
 * Called from the TX task context, not directly from application code.
 */

#pragma once

#include "Rf433InternalTypes.h"

namespace Rf433TransmissionHandler {

/**
 * @brief Initialize RCSwitch radio hardware
 * 
 * Sets up TX pin, default protocol, and repeat count.
 * Must be called once from TX task before transmitting.
 */
void init();

/**
 * @brief Check if radio is initialized and ready
 */
bool isReady();

/**
 * @brief Execute a transmission request
 * 
 * Configures radio parameters and sends the RF frame.
 * Blocks until transmission completes.
 * 
 * @param request Transmission parameters (protocol, pulse length, repeat, etc.)
 * 
 * @note This is a blocking operation. Call only from TX task.
 * @note Power management (on/off) is handled by caller.
 */
void transmit(const Rf433Internal::TxRequest& request);

}  // namespace Rf433TransmissionHandler
