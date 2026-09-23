#pragma once

#include <PsychicHttpServer.h>
#include <security/SecurityManager.h>

namespace API {

class SensorsApiService {
public:
    SensorsApiService(PsychicHttpServer* server, SecurityManager* securityManager);
    void begin();

private:
    esp_err_t handleGetSensors(PsychicRequest* request);

    PsychicHttpServer* _server;
    SecurityManager* _securityManager;
};

}  // namespace API
