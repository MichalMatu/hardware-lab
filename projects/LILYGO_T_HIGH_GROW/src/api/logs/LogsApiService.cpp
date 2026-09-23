#include "LogsApiService.h"

#include "../../config/AppConfig.h"
#include "../../sensors/SensorLoggingTask.h"
#include "../../power/PowerManager.h"
#include "../../system/Logging.h"
#include <ArduinoJson.h>
#include <PsychicFileResponse.h>
#include <PsychicJson.h>
#include <LittleFS.h>

namespace API {

LogsApiService::LogsApiService(PsychicHttpServer* server, SecurityManager* securityManager)
    : _server(server), _securityManager(securityManager) {}

void LogsApiService::begin() {
    auto adminWrap = [this](auto fn) {
        return _securityManager->wrapRequest(
            [fn](PsychicRequest* request) -> esp_err_t {
                POWER::PowerManager::notifyActivity("api/logs");
                return fn(request);
            },
            AuthenticationPredicates::IS_ADMIN);
    };

    _server->on("/api/logs", HTTP_GET, adminWrap([this](PsychicRequest* request) { return handleList(request); }));

    _server->on("/api/logs/download", HTTP_GET, adminWrap([this](PsychicRequest* request) { return handleDownload(request); }));

    _server->on("/api/logs/delete", HTTP_DELETE, adminWrap([this](PsychicRequest* request) { return handleDelete(request); }));

    _server->on("/rest/logs/tail", HTTP_GET, adminWrap([this](PsychicRequest* request) { return handleTail(request); }));
}

esp_err_t LogsApiService::handleList(PsychicRequest* request) {
    PsychicJsonResponse response(request);
    JsonVariant& jsonRoot = response.getRoot();
    JsonArray months = jsonRoot["months"].to<JsonArray>();

    File dataRoot = LittleFS.open("/data");
    if (dataRoot) {
        File dir = dataRoot.openNextFile();
        while (dir) {
            if (dir.isDirectory()) {
                const char* monthPath = dir.path();
                const char* monthName = monthPath;
                if (monthPath) {
                    const char* slash = strrchr(monthPath, '/');
                    if (slash && *(slash + 1) != '\0') {
                        monthName = slash + 1;
                    }
                }

                JsonObject month = months.add<JsonObject>();
                month["path"] = monthPath ? monthPath : "/data";
                month["name"] = monthName ? monthName : "";
                JsonArray files = month["files"].to<JsonArray>();

                File monthDir = LittleFS.open(monthPath ? monthPath : "/data");
                if (monthDir) {
                    File file = monthDir.openNextFile();
                    while (file) {
                        if (!file.isDirectory()) {
                            const char* fullName = file.name();
                            const char* baseName = fullName;
                            if (fullName) {
                                const char* slash = strrchr(fullName, '/');
                                if (slash && *(slash + 1) != '\0') {
                                    baseName = slash + 1;
                                }
                            }

                            JsonObject fileObj = files.add<JsonObject>();
                            fileObj["name"] = baseName ? baseName : "";
                            fileObj["size"] = file.size();
                        }

                        file.close();
                        file = monthDir.openNextFile();
                    }
                    monthDir.close();
                }

                // Remove empty months to keep payload small.
                if (files.size() == 0) {
                    months.remove(months.size() - 1);
                }
            }

            dir.close();
            dir = dataRoot.openNextFile();
        }
        dataRoot.close();
    }

    return response.send();
}

esp_err_t LogsApiService::handleDownload(PsychicRequest* request) {
    if (!request->hasParam("file")) {
        return request->reply(400, "text/plain", "Missing file parameter");
    }

    String filename = request->getParam("file")->value().c_str();

    // Safety: this endpoint is intended only for log files under /data.
    if (!filename.startsWith("/data/")) {
        return request->reply(400, "text/plain", "Invalid file path");
    }

    SemaphoreHandle_t fsMutex = SensorLoggingTask::getFsMutex();
    if (!fsMutex || !xSemaphoreTake(fsMutex, pdMS_TO_TICKS(API::FS_MUTEX_TIMEOUT_MS))) {
        return request->reply(503, "text/plain", "Filesystem busy, try again");
    }

    if (!LittleFS.exists(filename)) {
        xSemaphoreGive(fsMutex);
        return request->reply(404, "text/plain", "File not found");
    }

    File file = LittleFS.open(filename, "r");
    if (!file) {
        xSemaphoreGive(fsMutex);
        return request->reply(500, "text/plain", "Failed to open file");
    }

    // Stream the binary file directly (client-side parsing)
    // Keep the FS mutex for the duration of the transfer to avoid concurrent FS access.
    PsychicFileResponse response(request, file, filename, "application/octet-stream", true);
    esp_err_t err = response.send();
    xSemaphoreGive(fsMutex);
    return err;
}

esp_err_t LogsApiService::handleDelete(PsychicRequest* request) {
    if (!request->hasParam("file")) {
        return request->reply(400, "text/plain", "Missing file parameter");
    }

    String filename = request->getParam("file")->value().c_str();

    SemaphoreHandle_t fsMutex = SensorLoggingTask::getFsMutex();
    if (!fsMutex || !xSemaphoreTake(fsMutex, pdMS_TO_TICKS(API::FS_MUTEX_TIMEOUT_MS))) {
        return request->reply(503, "text/plain", "Filesystem busy, try again");
    }

    if (!LittleFS.exists(filename)) {
        xSemaphoreGive(fsMutex);
        return request->reply(404, "text/plain", "File not found");
    }

    bool deleted = LittleFS.remove(filename);
    xSemaphoreGive(fsMutex);

    if (!deleted) {
        return request->reply(500, "text/plain", "Failed to delete file");
    }

    return request->reply(200, "text/plain", "File deleted successfully");
}

esp_err_t LogsApiService::handleTail(PsychicRequest* request) {
    uint16_t lines = 50;
    if (request->hasParam("lines")) {
        lines = static_cast<uint16_t>(request->getParam("lines")->value().toInt());
        if (lines == 0) {
            lines = 1;
        }
    }

    PsychicJsonResponse response(request);
    JsonVariant& jsonRoot = response.getRoot();
    JsonArray arr = jsonRoot["lines"].to<JsonArray>();
    auto tail = LOG::Logging::tail(lines);
    for (const auto& line : tail) {
        const char* lvl = "N";
        switch (line.levelChar) {
            case 'E': lvl = "E"; break;
            case 'W': lvl = "W"; break;
            case 'I': lvl = "I"; break;
            case 'D': lvl = "D"; break;
            case 'V': lvl = "V"; break;
            default: break;
        }

        JsonObject obj = arr.add<JsonObject>();
        obj["timestampMs"] = line.timestampMs;
        obj["level"] = lvl;
        obj["tag"] = line.tag ? line.tag : "";
        obj["message"] = line.message;
    }

    return response.send();
}

}  // namespace API
