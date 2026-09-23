#include "SensorLoggingTask.h"

#include "../config/AppConfig.h"
#include "../system/Logging.h"

#include "hw/SensorHardware.h"
#include "runtime/SensorCommandQueue.h"
#include "runtime/SensorState.h"
#include "readers/SensorReadPipeline.h"
#include "logging/SensorCsvLogger.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#undef LOG_TAG
#define LOG_TAG "Sensor"

TaskHandle_t SensorLoggingTask::_taskHandle = nullptr;
bool SensorLoggingTask::_initialized = false;
uint32_t SensorLoggingTask::_lastReadTime_ms = 0;
uint32_t SensorLoggingTask::_lastLogTime_ms = 0;

void SensorLoggingTask::begin() {
    if (!ensureInitialized()) {
        LOGE("Init failed");
        return;
    }
    
    // Create task pinned to core 1
    BaseType_t result = xTaskCreatePinnedToCore(
        taskLoop,
        "SensorLogger",
        SENSOR::STACK_SIZE,
        nullptr,
        SENSOR::TASK_PRIORITY,
        &_taskHandle,
        SENSOR::TASK_CORE
    );
    
    if (result != pdPASS) {
        LOGE("Failed to create task");
        return;
    }
    
    LOGI("Task started on core 1");
}

bool SensorLoggingTask::ensureInitialized() {
    if (_initialized) {
        return true;
    }

    if (!SENSORS::SensorState::ensureInitialized()) {
        return false;
    }
    if (!SENSORS::SensorCommandQueue::ensureInitialized()) {
        return false;
    }
    if (!SENSORS::SensorHardware::ensureInitialized()) {
        return false;
    }

    _initialized = true;
    return true;
}

bool SensorLoggingTask::singleShotReadAndLog() {
    if (!ensureInitialized()) {
        return false;
    }

    LOGI("TASK: READ_START");
    SensorSnapshot snap;
    PhaseStatus readStatus;
    SENSORS::SensorReadPipeline::readAll(snap, readStatus);

    // Keep seq monotonically increasing
    SensorSnapshot prev = SENSORS::SensorState::getSnapshot();
    snap.seq = prev.seq + 1;

    SENSORS::SensorState::updateAfterRead(snap, readStatus);

    if (readStatus.ok) {
        LOGI("TASK: READ_OK (%.0f ms)", (float)readStatus.duration_ms);
    } else {
        LOGE("TASK: READ_FAIL (%.0f ms, code=%s)",
             (float)readStatus.duration_ms, readStatus.error_code.c_str());
        return false;
    }

    LOGI("TASK: WRITE_START");
    PhaseStatus writeStatus;
    SENSORS::SensorBinaryLogger::writeSnapshot(snap, writeStatus);
    SENSORS::SensorState::updateAfterWrite(writeStatus);

    if (writeStatus.ok) {
        LOGI("TASK: WRITE_OK (%.0f ms)", (float)writeStatus.duration_ms);
    } else {
        LOGE("TASK: WRITE_FAIL (%.0f ms, code=%s)",
             (float)writeStatus.duration_ms, writeStatus.error_code.c_str());
    }

    return writeStatus.ok;
}

void SensorLoggingTask::sendCommand(SensorTaskCommand cmd) {
    SENSORS::SensorCommandQueue::send(cmd);
}

SensorSnapshot SensorLoggingTask::getSnapshot() {
    return SENSORS::SensorState::getSnapshot();
}

PhaseStatus SensorLoggingTask::getLastReadStatus() {
    return SENSORS::SensorState::getLastReadStatus();
}

PhaseStatus SensorLoggingTask::getLastWriteStatus() {
    return SENSORS::SensorState::getLastWriteStatus();
}

SensorSnapshot SensorLoggingTask::getLastGoodSnapshot() {
    return SENSORS::SensorState::getLastGoodSnapshot();
}

ErrorInfo SensorLoggingTask::getLastErrorInfo() {
    return SENSORS::SensorState::getLastErrorInfo();
}

SemaphoreHandle_t SensorLoggingTask::getFsMutex() {
    return SENSORS::SensorState::getFsMutex();
}

void SensorLoggingTask::taskLoop(void* parameter) {
    LOGI("Loop started (on-demand mode)");
    
    _lastReadTime_ms = 0;  // No initial read
    _lastLogTime_ms = millis();
    
    while (true) {
        uint32_t now = millis();
        bool forceRead = false;
        bool forceLog = false;
        
        // Check for commands
        SensorTaskCommand cmd;
        if (SENSORS::SensorCommandQueue::tryReceive(cmd)) {
            LOGD("Command received: %d", cmd);
            if (cmd == CMD_FORCE_READ || cmd == CMD_FORCE_READ_AND_LOG) {
                forceRead = true;
            }
            if (cmd == CMD_FORCE_LOG || cmd == CMD_FORCE_READ_AND_LOG) {
                forceLog = true;
            }
        }
        
        // ON-DEMAND mode: read only when:
        // 1. Explicitly requested (CMD_FORCE_READ)
        // 2. Before periodic logging (to get fresh data)
        bool periodicLogDue = (now - _lastLogTime_ms >= SENSOR::LOG_INTERVAL_MS);
        bool shouldRead = forceRead || (periodicLogDue && !forceLog);
        
        if (shouldRead) {
            LOGI("TASK: READ_START");
            SensorSnapshot snap;
            PhaseStatus readStatus;
            SENSORS::SensorReadPipeline::readAll(snap, readStatus);

            SensorSnapshot prev = SENSORS::SensorState::getSnapshot();
            snap.seq = prev.seq + 1;
            SENSORS::SensorState::updateAfterRead(snap, readStatus);

            if (readStatus.ok) {
                LOGI("TASK: READ_OK (%.0f ms)", (float)readStatus.duration_ms);
            } else {
                LOGE("TASK: READ_FAIL (%.0f ms, code=%s)",
                     (float)readStatus.duration_ms, readStatus.error_code.c_str());
            }
            _lastReadTime_ms = now;
        }
        
        // Periodic or forced log (only after successful read)
        bool shouldLog = forceLog || periodicLogDue;
        if (shouldLog) {
            LOGI("TASK: WRITE_START");
            SensorSnapshot snap = SENSORS::SensorState::getSnapshot();

            PhaseStatus writeStatus;
            SENSORS::SensorBinaryLogger::writeSnapshot(snap, writeStatus);
            SENSORS::SensorState::updateAfterWrite(writeStatus);

            if (writeStatus.ok) {
                LOGI("TASK: WRITE_OK (%.0f ms)", (float)writeStatus.duration_ms);
            } else {
                LOGE("TASK: WRITE_FAIL (%.0f ms, code=%s)",
                     (float)writeStatus.duration_ms, writeStatus.error_code.c_str());
            }
            _lastLogTime_ms = now;
        }
        
        // Low power: longer sleep when idle
        vTaskDelay(pdMS_TO_TICKS(SENSOR::TASK_LOOP_SLEEP_MS));  // 500ms instead of 100ms
    }
}
