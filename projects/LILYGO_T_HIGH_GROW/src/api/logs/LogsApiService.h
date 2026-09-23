#pragma once

#include <PsychicHttpServer.h>
#include <security/SecurityManager.h>

namespace API {

class LogsApiService {
public:
    LogsApiService(PsychicHttpServer* server, SecurityManager* securityManager);
    void begin();

private:
    esp_err_t handleList(PsychicRequest* request);
    esp_err_t handleDownload(PsychicRequest* request);
    esp_err_t handleDelete(PsychicRequest* request);
    esp_err_t handleTail(PsychicRequest* request);

    PsychicHttpServer* _server;
    SecurityManager* _securityManager;
};

}  // namespace API
