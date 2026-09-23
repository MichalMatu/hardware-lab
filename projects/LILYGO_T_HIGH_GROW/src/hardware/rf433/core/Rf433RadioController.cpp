/**
 * @file Rf433RadioController.cpp
 * @brief RF433 public API - thin wrapper over task controller
 * 
 * Provides backward-compatible public API for RF433 transmissions.
 * Delegates to modular subsystems:
 * - Rf433TaskController: task/queue management
 * - Rf433PowerManager: power control
 * - Rf433TransmissionHandler: RCSwitch operations
 */

#include "Rf433RadioController.h"

#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "../../../system/Logging.h"
#include "Rf433InternalTypes.h"
#include "Rf433TaskController.h"

#undef LOG_TAG
#define LOG_TAG "RF433API"

namespace Rf433RadioController {

void init() {
    Rf433TaskController::init();
}

bool isReady() {
    return Rf433TaskController::isReady();
}

bool sendFrame(uint32_t value,
               uint8_t bitLength,
               const TxConfig& config,
               const char* logLabel) {
    if (!Rf433TaskController::isReady()) {
        LOGW("Not initialized, dropping frame");
        return false;
    }
    
    // Build internal request
    Rf433Internal::TxRequest req{};
    req.value = value;
    req.bitLength = bitLength;
    req.protocol = config.protocol;
    req.repeat = config.repeat;
    req.pulseLength = config.pulseLength;
    req.completionSemaphore = nullptr;  // Async - no wait
    
    if (logLabel != nullptr) {
        strncpy(req.logLabel, logLabel, Rf433Internal::kLogLabelMaxLen - 1);
        req.logLabel[Rf433Internal::kLogLabelMaxLen - 1] = '\0';
    } else {
        req.logLabel[0] = '\0';
    }
    
    // Enqueue (non-blocking)
    return Rf433TaskController::enqueueRequest(req);
}

bool sendFrameSync(uint32_t value,
                   uint8_t bitLength,
                   const TxConfig& config,
                   const char* logLabel,
                   uint32_t timeoutMs) {
    if (!Rf433TaskController::isReady()) {
        LOGW("Not initialized, cannot send sync");
        return false;
    }
    
    // Create semaphore for completion notification
    SemaphoreHandle_t completionSem = xSemaphoreCreateBinary();
    if (completionSem == nullptr) {
        LOGE("Failed to create completion semaphore");
        return false;
    }
    
    // Build internal request
    Rf433Internal::TxRequest req{};
    req.value = value;
    req.bitLength = bitLength;
    req.protocol = config.protocol;
    req.repeat = config.repeat;
    req.pulseLength = config.pulseLength;
    req.completionSemaphore = completionSem;
    
    if (logLabel != nullptr) {
        strncpy(req.logLabel, logLabel, Rf433Internal::kLogLabelMaxLen - 1);
        req.logLabel[Rf433Internal::kLogLabelMaxLen - 1] = '\0';
    } else {
        req.logLabel[0] = '\0';
    }
    
    // Enqueue with timeout (for sync operations)
    if (!Rf433TaskController::enqueueRequestWithTimeout(req, 100)) {
        LOGW("Queue full for sync send: %s", req.logLabel[0] ? req.logLabel : "unnamed");
        vSemaphoreDelete(completionSem);
        return false;
    }
    
    // Wait for completion
    bool success = (xSemaphoreTake(completionSem, pdMS_TO_TICKS(timeoutMs)) == pdTRUE);
    vSemaphoreDelete(completionSem);
    
    if (!success) {
        LOGW("Sync send timeout: %s", req.logLabel[0] ? req.logLabel : "unnamed");
    }
    
    return success;
}

}  // namespace Rf433RadioController
