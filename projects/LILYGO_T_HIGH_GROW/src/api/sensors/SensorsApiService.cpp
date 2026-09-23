#include "SensorsApiService.h"

#include "../../config/AppConfig.h"
#include "../../sensors/SensorLoggingTask.h"
#include "../../power/PowerManager.h"
#include "../../system/Logging.h"
#include <ArduinoJson.h>
#include <PsychicJson.h>

#undef LOG_TAG
#define LOG_TAG "ApiSensors"

namespace API {

SensorsApiService::SensorsApiService(PsychicHttpServer* server, SecurityManager* securityManager)
    : _server(server), _securityManager(securityManager) {}

void SensorsApiService::begin() {
    _server->on("/api/sensors", HTTP_GET, [this](PsychicRequest* request) {
        POWER::PowerManager::notifyActivity("api/sensors");
        return handleGetSensors(request);
    });
}

esp_err_t SensorsApiService::handleGetSensors(PsychicRequest* request) {
    bool forceRead = request->hasParam("force");

    SensorSnapshot snap = SensorLoggingTask::getSnapshot();
    uint32_t dataAge = millis() - snap.timestamp_ms;
    constexpr uint32_t kMaxDataAgeMs = API::SENSOR_MAX_AGE_MS;

    if (forceRead || dataAge > kMaxDataAgeMs || snap.timestamp_ms == 0) {
        LOGI("Data stale, triggering fresh read...");
        SensorLoggingTask::sendCommand(CMD_FORCE_READ);

        uint32_t waitStart = millis();
        uint32_t oldSeq = snap.seq;
        while (millis() - waitStart < API::SENSOR_DATA_WAIT_MS) {
            vTaskDelay(pdMS_TO_TICKS(50));
            snap = SensorLoggingTask::getSnapshot();
            if (snap.seq > oldSeq) {
                LOGI("Fresh data ready");
                break;
            }
        }
    }

    PhaseStatus readStatus = SensorLoggingTask::getLastReadStatus();
    SensorSnapshot lastGood = SensorLoggingTask::getLastGoodSnapshot();
    ErrorInfo lastError = SensorLoggingTask::getLastErrorInfo();

    PsychicJsonResponse response(request);
    JsonVariant& root = response.getRoot();
    root["lux"] = snap.lux;
    root["temp"] = snap.temp;
    root["humid"] = snap.humid;
    root["soil"] = snap.soil;
    root["salt"] = snap.salt;
    root["batPerc"] = snap.batPerc;
    root["batVolt"] = snap.batVolt;
    root["timestamp_ms"] = snap.timestamp_ms;
    root["seq"] = snap.seq;

    uint32_t age_ms = millis() - snap.timestamp_ms;
    root["age_ms"] = age_ms;
    root["age_sec"] = age_ms / 1000;

    root["lastReadOk"] = readStatus.ok;
    if (!readStatus.error_code.isEmpty()) {
        root["lastError"] = readStatus.error_code;
    }

    root["lastGoodSeq"] = lastGood.seq;
    root["lastGoodTimestamp_ms"] = lastGood.timestamp_ms;

    if (!lastError.code.isEmpty()) {
        JsonObject err = root["lastErrorInfo"].to<JsonObject>();
        err["code"] = lastError.code;
        err["timestamp_ms"] = lastError.timestamp_ms;
    }

    return response.send();
}

}  // namespace API
