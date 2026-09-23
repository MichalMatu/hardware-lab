/**
 * @file Rf433Manager.cpp
 * @brief RF433 manager facade implementation
 * 
 * Coordinates subsystems: registry, state tracking, command dispatch.
 */

#include "Rf433Manager.h"
#include "Rf433DeviceRegistry.h"
#include "Rf433StateTracker.h"
#include "Rf433CommandDispatcher.h"
#include "../../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "RF433Mgr"

Rf433Manager::Rf433Manager()
    : _registry(new Rf433DeviceRegistry()),
      _stateTracker(new Rf433StateTracker()),
      _dispatcher(new Rf433CommandDispatcher(_registry.get(), _stateTracker.get())) {
    LOGD("Manager initialized");
}

Rf433Manager::~Rf433Manager() {
    // unique_ptr handles cleanup
}

// ============================================================================
// Device CRUD (delegates to DeviceRegistry)
// ============================================================================

bool Rf433Manager::addDevice(const Rf433Device& device) {
    return _registry->addDevice(device);
}

bool Rf433Manager::updateDevice(const String& id, const Rf433Device& device) {
    bool success = _registry->updateDevice(id, device);
    
    // If ID changed, update state tracking
    if (success && id != device.id) {
        _stateTracker->renameState(id, device.id);
    }
    
    return success;
}

bool Rf433Manager::removeDevice(const String& id) {
    bool success = _registry->removeDevice(id);
    
    // Remove associated state
    if (success) {
        _stateTracker->removeState(id);
    }
    
    return success;
}

const Rf433Device* Rf433Manager::getDevice(const String& id) const {
    return _registry->getDevice(id);
}

const std::vector<Rf433Device>& Rf433Manager::getDevices() const {
    return _registry->getDevices();
}

void Rf433Manager::clearDevices() {
    _registry->clearDevices();
    _stateTracker->clearStates();
}

bool Rf433Manager::hasDevice(const String& id) const {
    return _registry->hasDevice(id);
}

size_t Rf433Manager::getDeviceCount() const {
    return _registry->getDeviceCount();
}

// ============================================================================
// Command dispatch (delegates to CommandDispatcher)
// ============================================================================

bool Rf433Manager::sendCommand(const String& deviceId, bool commandOn, bool async) {
    return _dispatcher->sendCommand(deviceId, commandOn, async);
}

// ============================================================================
// State tracking (delegates to StateTracker)
// ============================================================================

const Rf433DeviceState* Rf433Manager::getDeviceState(const String& deviceId) const {
    return _stateTracker->getState(deviceId);
}

const std::vector<Rf433DeviceState>& Rf433Manager::getDeviceStates() const {
    return _stateTracker->getAllStates();
}
