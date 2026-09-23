#pragma once

#include <PsychicHttpServer.h>
#include <security/SecurityManager.h>

namespace API {

class ConfigApiService {
public:
    ConfigApiService(PsychicHttpServer* server, SecurityManager* securityManager);
    void begin();

private:
    PsychicHttpServer* _server;
    SecurityManager* _securityManager;
};

}  // namespace API
