#include "ChartsApiService.h"

#include "../../config/AppConfig.h"
#include "../../sensors/SensorLoggingTask.h"
#include "../../power/PowerManager.h"
#include "../../system/ErrorCodes.h"
#include "../../datalogger/BinaryFormat.h"
#include <LittleFS.h>
#include <time.h>
#include <esp_http_server.h>

namespace API {

ChartsApiService::ChartsApiService(PsychicHttpServer* server, SecurityManager* securityManager)
    : _server(server), _securityManager(securityManager) {}

void ChartsApiService::begin() {
    _server->on("/api/charts", HTTP_GET, [this](PsychicRequest* request) {
        POWER::PowerManager::notifyActivity("api/charts");
        return handleCharts(request);
    });
}

esp_err_t ChartsApiService::handleCharts(PsychicRequest* request) {
    time_t now = time(nullptr);
    struct tm timeinfo {};
    if (localtime_r(&now, &timeinfo) == nullptr) {
        return request->reply(500, "application/json", "{\"error\":\"Time not available\"}");
    }
    char monthDir[8], dateFile[11];
    strftime(monthDir, sizeof(monthDir), "%Y-%m", &timeinfo);
    strftime(dateFile, sizeof(dateFile), "%Y-%m-%d", &timeinfo);

    char filename[48];
    snprintf(filename, sizeof(filename), "/data/%s/%s.bin", monthDir, dateFile);

    SemaphoreHandle_t fsMutex = SensorLoggingTask::getFsMutex();
    if (!fsMutex || !xSemaphoreTake(fsMutex, pdMS_TO_TICKS(API::FS_MUTEX_TIMEOUT_MS))) {
        String errorMsg = String("{\"error\":\"") + ErrorCodes::Busy::FILESYSTEM_BUSY + "\"}";
        return request->reply(503, "application/json", errorMsg.c_str());
    }

    if (!LittleFS.exists(filename)) {
        xSemaphoreGive(fsMutex);
        return request->reply(404, "application/json", "{\"error\":\"No data for today\"}");
    }

    File file = LittleFS.open(filename, "r");
    if (!file) {
        xSemaphoreGive(fsMutex);
        return request->reply(500, "application/json", "{\"error\":\"Failed to open file\"}");
    }

    // Stream raw binary file to client with metadata headers.
    // Avoid heap allocations inside PsychicFileResponse (8KB chunk malloc) to prevent intermittent 500s on a fragmented heap.
    httpd_req_t* rawReq = request->request();

    esp_err_t err = ESP_OK;
    err = httpd_resp_set_status(rawReq, "200 OK");
    if (err != ESP_OK) {
        file.close();
        xSemaphoreGive(fsMutex);
        return err;
    }

    err = httpd_resp_set_type(rawReq, "application/octet-stream");
    if (err != ESP_OK) {
        file.close();
        xSemaphoreGive(fsMutex);
        return err;
    }

    char recordSizeBuf[16];
    char versionBuf[16];
    char headerSizeBuf[16];

    snprintf(recordSizeBuf, sizeof(recordSizeBuf), "%d", DATALOG::BINARY_RECORD_SIZE);
    snprintf(versionBuf, sizeof(versionBuf), "%d", DATALOG::BINARY_VERSION);
    snprintf(headerSizeBuf, sizeof(headerSizeBuf), "%u", static_cast<unsigned>(sizeof(DATALOG::BinaryFileHeader)));

    httpd_resp_set_hdr(rawReq, "X-Record-Size", recordSizeBuf);
    httpd_resp_set_hdr(rawReq, "X-Format-Version", versionBuf);
    httpd_resp_set_hdr(rawReq, "X-File-Header-Size", headerSizeBuf);

    // Optional: keep consistent disposition without embedding filename (avoids any risk of invalid chars)
    httpd_resp_set_hdr(rawReq, "Content-Disposition", "inline");

    uint8_t buffer[1024];
    while (true) {
        const size_t readLen = file.read(buffer, sizeof(buffer));
        if (readLen == 0) {
            break;
        }
        err = httpd_resp_send_chunk(rawReq, reinterpret_cast<const char*>(buffer), readLen);
        if (err != ESP_OK) {
            break;
        }
    }

    file.close();
    xSemaphoreGive(fsMutex);

    if (err == ESP_OK) {
        // Signal end of chunked response
        err = httpd_resp_send_chunk(rawReq, nullptr, 0);
    }
    return err;
}

}  // namespace API
