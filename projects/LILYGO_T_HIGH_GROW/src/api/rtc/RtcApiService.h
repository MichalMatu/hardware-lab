#pragma once

#include <PsychicHttpServer.h>
#include <security/SecurityManager.h>

namespace API {

class RtcApiService {
public:
    RtcApiService(PsychicHttpServer* server, SecurityManager* securityManager);
    void begin();

private:
    esp_err_t handleSync(PsychicRequest* request);
    esp_err_t handleStatus(PsychicRequest* request);

    PsychicHttpServer* _server;
    SecurityManager* _securityManager;
};

}  // namespace API
