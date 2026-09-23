/**
 * @file Rf433TaskController.h
 * @brief FreeRTOS task and queue management for RF433 transmissions
 * 
 * Manages the dedicated TX task and request queue:
 * - Task lifecycle (create, monitor)
 * - Queue operations (enqueue requests)
 * - Task execution loop
 */

#pragma once

#include "Rf433InternalTypes.h"
#include <cstdint>

namespace Rf433TaskController {

/**
 * @brief Initialize TX task and queue
 * 
 * Creates FreeRTOS task pinned to CPU1 and request queue.
 * Must be called once during system startup.
 * 
 * @return true on success, false on task/queue creation failure
 */
bool init();

/**
 * @brief Check if task and queue are ready
 */
bool isReady();

/**
 * @brief Enqueue a transmission request (non-blocking)
 * 
 * @param request TX parameters and optional completion semaphore
 * @return true if queued, false if queue full or not initialized
 */
bool enqueueRequest(const Rf433Internal::TxRequest& request);

/**
 * @brief Enqueue with timeout (for sync operations)
 * 
 * @param request TX parameters
 * @param timeoutMs Maximum time to wait for queue space
 * @return true if queued, false on timeout or not initialized
 */
bool enqueueRequestWithTimeout(const Rf433Internal::TxRequest& request, uint32_t timeoutMs);

}  // namespace Rf433TaskController
