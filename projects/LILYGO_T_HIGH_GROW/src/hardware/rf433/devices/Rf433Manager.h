/**
 * @file Rf433Manager.h
 * @brief RF433 high-level coordinator (facade)
 * 
 * Provides unified interface for RF433 operations by coordinating:
 * - Rf433DeviceRegistry: device CRUD
 * - Rf433StateTracker: device state tracking
 * - Rf433CommandDispatcher: command transmission
 * 
 * Maintains backward compatibility with existing API.
 */

#pragma once

#include <Arduino.h>
#include <vector>
#include <memory>
#include "../../../config/Rf433Config.h"

// Forward declarations
class Rf433DeviceRegistry;
class Rf433StateTracker;
class Rf433CommandDispatcher;

/**
 * @brief Facade for RF433 device management and operations
 * 
 * Coordinates device registry, state tracking, and command dispatch.
 * Thread-safe access (called from HTTP handlers).
 */
class Rf433Manager {
public:
    Rf433Manager();
    ~Rf433Manager();
    
    // ========================================================================
    // Device CRUD (delegates to DeviceRegistry)
    // ========================================================================
    
    /**
     * @brief Add a new device to the registry
     * @return true if added, false if duplicate ID or limit reached
     */
    bool addDevice(const Rf433Device& device);
    
    /**
     * @brief Update an existing device
     * @return true if updated, false if not found
     */
    bool updateDevice(const String& id, const Rf433Device& device);
    
    /**
     * @brief Remove a device by ID
     * @return true if removed, false if not found
     */
    bool removeDevice(const String& id);
    
    /**
     * @brief Get a device by ID
     * @return Pointer to device or nullptr if not found
     */
    const Rf433Device* getDevice(const String& id) const;
    
    /**
     * @brief Get all devices
     */
    const std::vector<Rf433Device>& getDevices() const;
    
    /**
     * @brief Clear all devices
     */
    void clearDevices();
    
    /**
     * @brief Check if device exists
     */
    bool hasDevice(const String& id) const;
    
    /**
     * @brief Get device count
     */
    size_t getDeviceCount() const;
    
    // ========================================================================
    // Command dispatch (delegates to CommandDispatcher)
    // ========================================================================
    
    /**
     * @brief Send ON/OFF command to a device
     * @param deviceId Device identifier
     * @param commandOn true = send ON code, false = send OFF code
     * @param async true = queue and return immediately, false = wait for completion
     * @return true if command sent/queued, false on error
     */
    bool sendCommand(const String& deviceId, bool commandOn, bool async = true);
    
    // ========================================================================
    // State tracking (delegates to StateTracker)
    // ========================================================================
    
    /**
     * @brief Get last command state for a device
     * @return Pointer to state or nullptr if no history
     */
    const Rf433DeviceState* getDeviceState(const String& deviceId) const;
    
    /**
     * @brief Get all device states
     */
    const std::vector<Rf433DeviceState>& getDeviceStates() const;
    
private:
    std::unique_ptr<Rf433DeviceRegistry> _registry;
    std::unique_ptr<Rf433StateTracker> _stateTracker;
    std::unique_ptr<Rf433CommandDispatcher> _dispatcher;
};
