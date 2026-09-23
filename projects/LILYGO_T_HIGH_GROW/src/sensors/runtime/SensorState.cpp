#include "SensorState.h"

#include "../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace SENSORS {

SemaphoreHandle_t SensorState::_snapshotMutex = nullptr;
SemaphoreHandle_t SensorState::_fsMutex = nullptr;

SensorSnapshot SensorState::_latestSnapshot;
SensorSnapshot SensorState::_lastGoodSnapshot;
PhaseStatus SensorState::_lastReadStatus;
PhaseStatus SensorState::_lastWriteStatus;
ErrorInfo SensorState::_lastErrorInfo;

bool SensorState::_initialized = false;

bool SensorState::ensureInitialized() {
    if (_initialized) {
        return true;
    }

    if (!_snapshotMutex) {
        _snapshotMutex = xSemaphoreCreateMutex();
    }
    if (!_fsMutex) {
        _fsMutex = xSemaphoreCreateMutex();
    }

    if (!_snapshotMutex || !_fsMutex) {
        LOGE("STATE: failed to create mutexes");
        return false;
    }

    _initialized = true;
    return true;
}

SensorSnapshot SensorState::getSnapshot() {
    SensorSnapshot snap;
    if (_snapshotMutex && xSemaphoreTake(_snapshotMutex, pdMS_TO_TICKS(100))) {
        snap = _latestSnapshot;
        xSemaphoreGive(_snapshotMutex);
    }
    return snap;
}

SensorSnapshot SensorState::getLastGoodSnapshot() {
    SensorSnapshot snap;
    if (_snapshotMutex && xSemaphoreTake(_snapshotMutex, pdMS_TO_TICKS(100))) {
        snap = _lastGoodSnapshot;
        xSemaphoreGive(_snapshotMutex);
    }
    return snap;
}

PhaseStatus SensorState::getLastReadStatus() {
    PhaseStatus status;
    if (_snapshotMutex && xSemaphoreTake(_snapshotMutex, pdMS_TO_TICKS(100))) {
        status = _lastReadStatus;
        xSemaphoreGive(_snapshotMutex);
    }
    return status;
}

PhaseStatus SensorState::getLastWriteStatus() {
    PhaseStatus status;
    if (_snapshotMutex && xSemaphoreTake(_snapshotMutex, pdMS_TO_TICKS(100))) {
        status = _lastWriteStatus;
        xSemaphoreGive(_snapshotMutex);
    }
    return status;
}

ErrorInfo SensorState::getLastErrorInfo() {
    ErrorInfo info;
    if (_snapshotMutex && xSemaphoreTake(_snapshotMutex, pdMS_TO_TICKS(100))) {
        info = _lastErrorInfo;
        xSemaphoreGive(_snapshotMutex);
    }
    return info;
}

void SensorState::updateAfterRead(const SensorSnapshot& snap, const PhaseStatus& status) {
    if (!_snapshotMutex) {
        return;
    }

    if (!xSemaphoreTake(_snapshotMutex, pdMS_TO_TICKS(100))) {
        return;
    }

    _latestSnapshot = snap;
    _lastReadStatus = status;

    if (status.ok) {
        _lastGoodSnapshot = snap;
    } else if (!status.error_code.isEmpty()) {
        _lastErrorInfo.code = status.error_code;
        _lastErrorInfo.timestamp_ms = millis();
    }

    xSemaphoreGive(_snapshotMutex);
}

void SensorState::updateAfterWrite(const PhaseStatus& status) {
    if (!_snapshotMutex) {
        return;
    }

    if (xSemaphoreTake(_snapshotMutex, pdMS_TO_TICKS(100))) {
        _lastWriteStatus = status;
        xSemaphoreGive(_snapshotMutex);
    }
}

SemaphoreHandle_t SensorState::getSnapshotMutex() {
    return _snapshotMutex;
}

SemaphoreHandle_t SensorState::getFsMutex() {
    return _fsMutex;
}

}  // namespace SENSORS
