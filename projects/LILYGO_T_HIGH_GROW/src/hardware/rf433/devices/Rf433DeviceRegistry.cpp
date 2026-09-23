/**
 * @file Rf433DeviceRegistry.cpp
 * @brief Device registry implementation
 */

#include "Rf433DeviceRegistry.h"
#include "../../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "RF433Registry"

Rf433DeviceRegistry::Rf433DeviceRegistry() {
    // Reserve space for devices to avoid reallocations
    _devices.reserve(RF433::MAX_DEVICES);
}

bool Rf433DeviceRegistry::addDevice(const Rf433Device& device) {
    // Validate device
    if (!device.isValid()) {
        LOGW("Add rejected: invalid device (id=%s)", device.id.c_str());
        return false;
    }
    
    // Check for duplicate ID
    if (hasDevice(device.id)) {
        LOGW("Add rejected: duplicate ID (id=%s)", device.id.c_str());
        return false;
    }
    
    // Check limit
    if (_devices.size() >= RF433::MAX_DEVICES) {
        LOGW("Add rejected: max devices reached (%u/%u)",
             static_cast<unsigned>(_devices.size()),
             static_cast<unsigned>(RF433::MAX_DEVICES));
        return false;
    }
    
    // Add device
    _devices.push_back(device);
    LOGD("Added device: id=%s label='%s' proto=%u bits=%u",
         device.id.c_str(),
         device.label.c_str(),
         device.protocol,
         device.bitLength);
    
    return true;
}

bool Rf433DeviceRegistry::updateDevice(const String& id, const Rf433Device& device) {
    // Validate new device data
    if (!device.isValid()) {
        LOGW("Update rejected: invalid device (id=%s)", id.c_str());
        return false;
    }
    
    // Find existing device
    int index = findDeviceIndex(id);
    if (index < 0) {
        LOGW("Update rejected: device not found (id=%s)", id.c_str());
        return false;
    }
    
    // If ID changed, check for conflicts
    if (id != device.id && hasDevice(device.id)) {
        LOGW("Update rejected: new ID already exists (old=%s new=%s)",
             id.c_str(), device.id.c_str());
        return false;
    }
    
    // Update device
    _devices[index] = device;
    LOGD("Updated device: id=%s label='%s'", device.id.c_str(), device.label.c_str());
    
    return true;
}

bool Rf433DeviceRegistry::removeDevice(const String& id) {
    int index = findDeviceIndex(id);
    if (index < 0) {
        LOGW("Remove rejected: device not found (id=%s)", id.c_str());
        return false;
    }
    
    // Remove device
    _devices.erase(_devices.begin() + index);
    LOGD("Removed device: id=%s", id.c_str());
    
    return true;
}

const Rf433Device* Rf433DeviceRegistry::getDevice(const String& id) const {
    int index = findDeviceIndex(id);
    return (index >= 0) ? &_devices[index] : nullptr;
}

void Rf433DeviceRegistry::clearDevices() {
    _devices.clear();
    LOGI("Cleared all devices");
}

bool Rf433DeviceRegistry::hasDevice(const String& id) const {
    return findDeviceIndex(id) >= 0;
}

int Rf433DeviceRegistry::findDeviceIndex(const String& id) const {
    for (size_t i = 0; i < _devices.size(); ++i) {
        if (_devices[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}
