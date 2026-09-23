/**
 * @file Rf433StateTracker.h
 * @brief Device state tracking for RF433 devices
 * 
 * Tracks last command state (ON/OFF) and transmission timestamp
 * for each device. Used for UI display and command history.
 */

#pragma once

#include <vector>
#include "../../../config/Rf433Config.h"

/**
 * @brief State tracker for RF433 device transmissions
 * 
 * Records last command (ON/OFF) and timestamp for each device.
 * Provides quick lookup by device ID.
 */
class Rf433StateTracker {
public:
    Rf433StateTracker();
    
    /**
     * @brief Update state after successful transmission
     * 
     * Records the command (ON/OFF) and current timestamp.
     * Creates new state entry if device hasn't been tracked yet.
     * 
     * @param deviceId Device identifier
     * @param commandOn true = ON command, false = OFF command
     */
    void updateState(const String& deviceId, bool commandOn);
    
    /**
     * @brief Get last command state for a device
     * @return Pointer to state or nullptr if no history
     */
    const Rf433DeviceState* getState(const String& deviceId) const;
    
    /**
     * @brief Get all device states
     */
    const std::vector<Rf433DeviceState>& getAllStates() const { return _deviceStates; }
    
    /**
     * @brief Clear all state tracking
     */
    void clearStates();
    
    /**
     * @brief Remove state for a specific device
     * 
     * Called when device is removed from registry.
     */
    void removeState(const String& deviceId);
    
    /**
     * @brief Update device ID in state tracking
     * 
     * Called when device ID changes (via update operation).
     */
    void renameState(const String& oldId, const String& newId);
    
private:
    std::vector<Rf433DeviceState> _deviceStates;
    
    // Find state index (returns -1 if not found)
    int findStateIndex(const String& deviceId) const;
};
