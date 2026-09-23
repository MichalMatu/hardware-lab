#include "RtcApiService.h"

#include "../../hardware/rtc/RTCModule.h"
#include "../../power/PowerManager.h"
#include <time.h>

namespace {
constexpr int kMinValidYear = 2025;
constexpr int kMaxValidYear = 2040;

bool isSystemTimeValid() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    int year = timeinfo.tm_year + 1900;
    return year >= kMinValidYear && year <= kMaxValidYear;
}
}

namespace API {

RtcApiService::RtcApiService(PsychicHttpServer* server, SecurityManager* securityManager)
    : _server(server), _securityManager(securityManager) {}

void RtcApiService::begin() {
    _server->on("/rest/rtc/sync", HTTP_POST,
        _securityManager->wrapRequest(
            [this](PsychicRequest *request) -> esp_err_t {
                POWER::PowerManager::notifyActivity("rest/rtc/sync");
                return handleSync(request);
            },
            AuthenticationPredicates::IS_ADMIN
        )
    );

    _server->on("/rest/rtc/status", HTTP_GET,
        _securityManager->wrapRequest(
            [this](PsychicRequest *request) -> esp_err_t {
                POWER::PowerManager::notifyActivity("rest/rtc/status");
                return handleStatus(request);
            },
            AuthenticationPredicates::IS_AUTHENTICATED
        )
    );
}

esp_err_t RtcApiService::handleSync(PsychicRequest* request) {
    if (!isSystemTimeValid()) {
        return request->reply(400, "application/json",
            "{\"status\":\"error\",\"message\":\"System time invalid (year must be 2025-2040). Set time via NTP or manually first.\"}");
    }

    bool success = RTCModule::syncWithSystemTime();
    if (success) {
        return request->reply(200, "application/json",
            "{\"status\":\"success\",\"message\":\"RTC synchronized with system time\"}");
    } else {
        return request->reply(400, "application/json",
            "{\"status\":\"error\",\"message\":\"Failed to sync RTC - check RTC module connection\"}");
    }
}

esp_err_t RtcApiService::handleStatus(PsychicRequest* request) {
    return RTCModule::getStatus(request);
}

}  // namespace API
