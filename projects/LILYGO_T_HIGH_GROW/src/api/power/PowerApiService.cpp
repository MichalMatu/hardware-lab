#include "PowerApiService.h"

#include "../../power/PowerManager.h"
#include <ArduinoJson.h>
#include <PsychicJson.h>

PowerApiService::PowerApiService(PsychicHttpServer* server, SecurityManager* securityManager)
    : _server(server), _securityManager(securityManager) {}

void PowerApiService::begin() {
    auto replyStatus = [](PsychicRequest *request) -> esp_err_t {
        POWER::PowerManager::notifyActivity("rest/power/status");
        auto reason = POWER::PowerManager::wakeReason();
        auto cfg = POWER::PowerManager::inactivityConfig();

        const char* reasonStr = "unknown";
        switch (reason) {
            case POWER::WakeReason::Timer: reasonStr = "timer"; break;
            case POWER::WakeReason::Button: reasonStr = "button"; break;
            case POWER::WakeReason::Other: reasonStr = "other"; break;
            default: break;
        }

        PsychicJsonResponse response(request);
        JsonVariant& root = response.getRoot();
        root["wake_reason"] = reasonStr;
        root["sleep_requested"] = POWER::PowerManager::isSleepRequested();
        root["sleep_eta_ms"] = POWER::PowerManager::sleepEtaMs();
        root["inactivity_timeout_ms"] = cfg.timeoutMs;
        root["grace_ms"] = cfg.graceAfterBootMs;
        root["wake_interval_ms"] = POWER::PowerManager::wakeIntervalMs();
        root["last_activity_ms"] = POWER::PowerManager::lastActivityMs();
        root["uptime_ms"] = millis();
        return response.send();
    };

    _server->on("/rest/power/status", HTTP_GET,
        _securityManager->wrapRequest(
            replyStatus,
            AuthenticationPredicates::IS_AUTHENTICATED
        )
    );

    auto sleepCycle = [](PsychicRequest *request) -> esp_err_t {
        POWER::PowerManager::notifyActivity("rest/power/sleepCycle");
        request->reply(200);

        // Enter deep sleep using the application power manager (timer wake + button wake).
        // This is intentionally different from the framework /rest/sleep endpoint.
        POWER::PowerManager::requestSleep("manual-sleepCycle");
        return ESP_OK;
    };

    _server->on("/rest/power/sleepCycle", HTTP_POST,
        _securityManager->wrapRequest(
            sleepCycle,
            AuthenticationPredicates::IS_AUTHENTICATED
        )
    );
}
