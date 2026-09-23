/**
 * @file Rf433CommandDispatcher.h
 * @brief Command dispatch logic for RF433 transmissions
 * 
 * Coordinates command execution:
 * - Validates device existence and command codes
 * - Prepares transmission configuration
 * - Dispatches to RadioController
 * - Updates state tracker on success
 */

#pragma once

#include <Arduino.h>

// Forward declarations
class Rf433DeviceRegistry;
class Rf433StateTracker;

/**
 * @brief Dispatcher for RF433 device commands
 * 
 * Handles command validation, transmission dispatch,
 * and state tracking updates.
 */
class Rf433CommandDispatcher {
public:
    /**
     * @brief Constructor
     * 
     * @param registry Device registry for lookups
     * @param stateTracker State tracker for history updates
     */
    Rf433CommandDispatcher(Rf433DeviceRegistry* registry, Rf433StateTracker* stateTracker);
    
    /**
     * @brief Send ON/OFF command to a device
     * 
     * Validates device, prepares transmission config, and dispatches
     * through RadioController. Updates state on success.
     * 
     * @param deviceId Device identifier
     * @param commandOn true = send ON code, false = send OFF code
     * @param async true = queue and return immediately, false = wait for completion
     * @return true if command sent/queued, false on error
     */
    bool sendCommand(const String& deviceId, bool commandOn, bool async = true);
    
private:
    Rf433DeviceRegistry* _registry;
    Rf433StateTracker* _stateTracker;
};
