/**
 * @file Rf433CommandDispatcher.cpp
 * @brief Command dispatcher implementation
 */

#include "Rf433CommandDispatcher.h"
#include "Rf433DeviceRegistry.h"
#include "Rf433StateTracker.h"
#include "../core/Rf433RadioController.h"
#include "../../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "RF433Dispatch"

Rf433CommandDispatcher::Rf433CommandDispatcher(Rf433DeviceRegistry* registry,
                                               Rf433StateTracker* stateTracker)
    : _registry(registry), _stateTracker(stateTracker) {
}

bool Rf433CommandDispatcher::sendCommand(const String& deviceId, bool commandOn, bool async) {
    // Find device
    const Rf433Device* device = _registry->getDevice(deviceId);
    if (device == nullptr) {
        LOGW("Send rejected: device not found (id=%s)", deviceId.c_str());
        return false;
    }
    
    // Select code (ON or OFF)
    uint32_t code = commandOn ? device->codeOn : device->codeOff;
    if (code == 0) {
        LOGW("Send rejected: code not set (id=%s cmd=%s)",
             deviceId.c_str(),
             commandOn ? "ON" : "OFF");
        return false;
    }
    
    // Check if RadioController is ready
    if (!Rf433RadioController::isReady()) {
        LOGW("Send rejected: RadioController not ready (id=%s)", deviceId.c_str());
        return false;
    }
    
    // Prepare transmission config
    Rf433RadioController::TxConfig config;
    config.protocol = device->protocol;
    config.repeat = device->repeat;
    config.pulseLength = device->pulseLength;
    
    // Build log label
    String logLabel = device->label + " [" + (commandOn ? "ON" : "OFF") + "]";
    
    // Dispatch transmission
    bool success;
    if (async) {
        success = Rf433RadioController::sendFrame(code, device->bitLength, config, logLabel.c_str());
    } else {
        success = Rf433RadioController::sendFrameSync(code, device->bitLength, config, logLabel.c_str());
    }
    
    if (success) {
        // Update state tracking
        _stateTracker->updateState(deviceId, commandOn);
        LOGI("Command queued: id=%s cmd=%s code=0x%08lX",
             deviceId.c_str(),
             commandOn ? "ON" : "OFF",
             static_cast<unsigned long>(code));
    } else {
        LOGW("Command failed: id=%s cmd=%s", deviceId.c_str(), commandOn ? "ON" : "OFF");
    }
    
    return success;
}
