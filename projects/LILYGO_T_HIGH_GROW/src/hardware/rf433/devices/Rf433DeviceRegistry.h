/**
 * @file Rf433DeviceRegistry.h
 * @brief Device registry (CRUD operations) for RF433 devices
 * 
 * Manages the list of configured RF433 devices:
 * - Add/update/remove/get devices
 * - Device validation and bounds checking
 * - Thread-safe for HTTP handler access
 */

#pragma once

#include <vector>
#include "../../../config/Rf433Config.h"

/**
 * @brief Registry for RF433 device definitions
 * 
 * Maintains the list of RF433 devices with CRUD operations.
 * Enforces MAX_DEVICES limit and validates device data.
 */
class Rf433DeviceRegistry {
public:
    Rf433DeviceRegistry();
    
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
    const std::vector<Rf433Device>& getDevices() const { return _devices; }
    
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
    size_t getDeviceCount() const { return _devices.size(); }
    
private:
    std::vector<Rf433Device> _devices;
    
    // Find device index (returns -1 if not found)
    int findDeviceIndex(const String& id) const;
};
