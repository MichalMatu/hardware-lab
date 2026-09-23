/**
 * @file Rf433TaskController.cpp
 * @brief FreeRTOS task controller implementation for RF433 TX
 */

#include "Rf433TaskController.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "../../../config/Rf433Config.h"
#include "../../../system/Logging.h"
#include "Rf433PowerManager.h"
#include "Rf433TransmissionHandler.h"

#undef LOG_TAG
#define LOG_TAG "RF433Task"

namespace {

QueueHandle_t g_txQueue = nullptr;
TaskHandle_t g_taskHandle = nullptr;

/**
 * @brief TX task main loop
 * 
 * Waits for requests in queue, manages power, and dispatches transmissions.
 */
void rf433TxTask(void* param) {
    (void)param;
    
    LOGI("TX task started (stack=%u priority=%u)",
         static_cast<unsigned>(RF433::TX_TASK_STACK_SIZE),
         static_cast<unsigned>(RF433::TX_TASK_PRIORITY));
    
    // Initialize transmission handler (RCSwitch)
    Rf433TransmissionHandler::init();
    
    for (;;) {
        Rf433Internal::TxRequest req;
        
        // Block waiting for TX request
        if (xQueueReceive(g_txQueue, &req, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        
        // Power on RF433 module (with stabilization delay)
        Rf433PowerManager::powerOn();
        
        // Perform transmission (blocking)
        Rf433TransmissionHandler::transmit(req);
        
        // Power off RF433 module (with delay to ensure transmission completed)
        Rf433PowerManager::powerOff();
        
        // Signal completion if synchronous send requested
        if (req.completionSemaphore != nullptr) {
            xSemaphoreGive(req.completionSemaphore);
        }
    }
}

}  // namespace

namespace Rf433TaskController {

bool init() {
    if (g_txQueue != nullptr) {
        LOGW("Already initialized");
        return true;
    }
    
    // Initialize power manager
    Rf433PowerManager::init();
    
    // Create queue for TX requests
    g_txQueue = xQueueCreate(RF433::TX_QUEUE_SIZE, sizeof(Rf433Internal::TxRequest));
    if (g_txQueue == nullptr) {
        LOGE("Failed to create TX queue");
        return false;
    }
    
    // Create dedicated TX task pinned to CPU1 to reduce interference
    BaseType_t result = xTaskCreatePinnedToCore(
        rf433TxTask,
        "rf433_tx",
        RF433::TX_TASK_STACK_SIZE,
        nullptr,
        RF433::TX_TASK_PRIORITY,
        &g_taskHandle,
        1  // Pin to CPU1 (APP_CPU)
    );
    
    if (result != pdPASS) {
        LOGE("Failed to create TX task");
        vQueueDelete(g_txQueue);
        g_txQueue = nullptr;
        return false;
    }
    
    LOGI("Initialized (queue=%u stack=%uB cpu=1)",
         static_cast<unsigned>(RF433::TX_QUEUE_SIZE),
         static_cast<unsigned>(RF433::TX_TASK_STACK_SIZE));
    
    return true;
}

bool isReady() {
    return g_txQueue != nullptr && g_taskHandle != nullptr;
}

bool enqueueRequest(const Rf433Internal::TxRequest& request) {
    if (g_txQueue == nullptr) {
        LOGW("Queue not initialized, dropping request");
        return false;
    }
    
    // Non-blocking queue send
    if (xQueueSend(g_txQueue, &request, 0) != pdTRUE) {
        LOGW("Queue full, dropping frame: %s",
             request.logLabel[0] ? request.logLabel : "unnamed");
        return false;
    }
    
    LOGD("Queued: %s value=0x%08lX",
         request.logLabel[0] ? request.logLabel : "unnamed",
         static_cast<unsigned long>(request.value));
    
    return true;
}

bool enqueueRequestWithTimeout(const Rf433Internal::TxRequest& request, uint32_t timeoutMs) {
    if (g_txQueue == nullptr) {
        LOGW("Queue not initialized, cannot enqueue");
        return false;
    }
    
    if (xQueueSend(g_txQueue, &request, pdMS_TO_TICKS(timeoutMs)) != pdTRUE) {
        LOGW("Queue send timeout for: %s",
             request.logLabel[0] ? request.logLabel : "unnamed");
        return false;
    }
    
    return true;
}

}  // namespace Rf433TaskController
