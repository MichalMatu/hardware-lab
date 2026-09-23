#pragma once

#include <PsychicHttpServer.h>
#include <security/SecurityManager.h>

class PowerApiService {
public:
    PowerApiService(PsychicHttpServer* server, SecurityManager* securityManager);
    void begin();

private:
    PsychicHttpServer* _server;
    SecurityManager* _securityManager;
};
