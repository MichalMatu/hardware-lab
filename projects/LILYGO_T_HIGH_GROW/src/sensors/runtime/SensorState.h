#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "../model/SensorTypes.h"

namespace SENSORS {

class SensorState {
public:
    static bool ensureInitialized();

    static SensorSnapshot getSnapshot();
    static SensorSnapshot getLastGoodSnapshot();
    static PhaseStatus getLastReadStatus();
    static PhaseStatus getLastWriteStatus();
    static ErrorInfo getLastErrorInfo();

    static void updateAfterRead(const SensorSnapshot& snap, const PhaseStatus& status);
    static void updateAfterWrite(const PhaseStatus& status);

    static SemaphoreHandle_t getSnapshotMutex();
    static SemaphoreHandle_t getFsMutex();

private:
    static SemaphoreHandle_t _snapshotMutex;
    static SemaphoreHandle_t _fsMutex;

    static SensorSnapshot _latestSnapshot;
    static SensorSnapshot _lastGoodSnapshot;
    static PhaseStatus _lastReadStatus;
    static PhaseStatus _lastWriteStatus;
    static ErrorInfo _lastErrorInfo;

    static bool _initialized;
};

}  // namespace SENSORS
