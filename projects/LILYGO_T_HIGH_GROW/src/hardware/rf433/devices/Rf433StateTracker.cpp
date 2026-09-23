/**
 * @file Rf433StateTracker.cpp
 * @brief State tracking implementation
 */

#include "Rf433StateTracker.h"
#include "../../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "RF433State"

Rf433StateTracker::Rf433StateTracker() {
    // Reserve space to match device limit
    _deviceStates.reserve(RF433::MAX_DEVICES);
}

void Rf433StateTracker::updateState(const String& deviceId, bool commandOn) {
    int index = findStateIndex(deviceId);
    if (index >= 0) {
        // Update existing state
        _deviceStates[index].lastCommandOn = commandOn;
        _deviceStates[index].lastTransmitMs = millis();
        LOGD("Updated state: id=%s cmd=%s", deviceId.c_str(), commandOn ? "ON" : "OFF");
    } else {
        // Create new state entry
        _deviceStates.emplace_back(deviceId, commandOn);
        LOGD("Created state: id=%s cmd=%s", deviceId.c_str(), commandOn ? "ON" : "OFF");
    }
}

const Rf433DeviceState* Rf433StateTracker::getState(const String& deviceId) const {
    int index = findStateIndex(deviceId);
    return (index >= 0) ? &_deviceStates[index] : nullptr;
}

void Rf433StateTracker::clearStates() {
    _deviceStates.clear();
    LOGD("Cleared all states");
}

void Rf433StateTracker::removeState(const String& deviceId) {
    int index = findStateIndex(deviceId);
    if (index >= 0) {
        _deviceStates.erase(_deviceStates.begin() + index);
        LOGD("Removed state: id=%s", deviceId.c_str());
    }
}

void Rf433StateTracker::renameState(const String& oldId, const String& newId) {
    int index = findStateIndex(oldId);
    if (index >= 0) {
        _deviceStates[index].deviceId = newId;
        LOGD("Renamed state: old=%s new=%s", oldId.c_str(), newId.c_str());
    }
}

int Rf433StateTracker::findStateIndex(const String& deviceId) const {
    for (size_t i = 0; i < _deviceStates.size(); ++i) {
        if (_deviceStates[i].deviceId == deviceId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}
