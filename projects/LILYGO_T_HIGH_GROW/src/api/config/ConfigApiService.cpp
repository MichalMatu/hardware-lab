#include "ConfigApiService.h"

#include "../../power/PowerManager.h"
#include <PsychicHttpServer.h>

// External config handlers (defined in SensorConfigHandlers.cpp)
extern esp_err_t handleGetConfig(PsychicRequest *request);
extern esp_err_t handleSaveConfig(PsychicRequest *request);

namespace API {

ConfigApiService::ConfigApiService(PsychicHttpServer* server, SecurityManager* securityManager)
    : _server(server), _securityManager(securityManager) {}

void ConfigApiService::begin() {
    _server->on(
        "/api/config",
        HTTP_GET,
        _securityManager->wrapRequest(
            [](PsychicRequest *request) -> esp_err_t {
                POWER::PowerManager::notifyActivity("api/config:get");
                return handleGetConfig(request);
            },
            AuthenticationPredicates::IS_ADMIN));

    _server->on(
        "/api/config",
        HTTP_POST,
        _securityManager->wrapRequest(
            [](PsychicRequest *request) -> esp_err_t {
                POWER::PowerManager::notifyActivity("api/config:post");
                return handleSaveConfig(request);
            },
            AuthenticationPredicates::IS_ADMIN));
}

}  // namespace API
