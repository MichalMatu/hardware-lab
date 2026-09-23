#pragma once

#include <PsychicHttpServer.h>
#include <security/SecurityManager.h>

namespace API {

class ChartsApiService {
public:
    ChartsApiService(PsychicHttpServer* server, SecurityManager* securityManager);
    void begin();

private:
    esp_err_t handleCharts(PsychicRequest* request);

    PsychicHttpServer* _server;
    SecurityManager* _securityManager;
};

}  // namespace API
