#include "SensorCsvLogger.h"

#include "../../config/AppConfig.h"
#include "../../datalogger/BinaryDataLogger.h"
#include "../../system/Logging.h"

#include "../runtime/SensorState.h"

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace SENSORS {

void SensorBinaryLogger::writeSnapshot(const SensorSnapshot& snap, PhaseStatus& outStatus) {
    outStatus.start_ms = millis();
    outStatus.ok = false;
    outStatus.error_code = "";
    outStatus.error_detail = "";

    SemaphoreHandle_t fsMutex = SensorState::getFsMutex();
    if (!fsMutex) {
        outStatus.error_code = "FS_MUTEX_NULL";
        outStatus.duration_ms = millis() - outStatus.start_ms;
        return;
    }

    if (!xSemaphoreTake(fsMutex, pdMS_TO_TICKS(API::FS_MUTEX_TIMEOUT_MS))) {
        LOGE("BIN: FS mutex timeout");
        outStatus.error_code = "FS_BUSY";
        outStatus.duration_ms = millis() - outStatus.start_ms;
        return;
    }

    LOGI("BIN: logging...");
    DATALOG::BinaryDataLogger::logSensorData(
        snap.temp, snap.humid, snap.lux,
        snap.soil, snap.salt,
        snap.batVolt, snap.batPerc
    );

    xSemaphoreGive(fsMutex);

    outStatus.duration_ms = millis() - outStatus.start_ms;
    outStatus.ok = true;
}

}  // namespace SENSORS
