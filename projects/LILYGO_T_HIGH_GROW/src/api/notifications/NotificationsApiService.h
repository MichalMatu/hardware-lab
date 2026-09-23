#pragma once

#include <PsychicHttpServer.h>
#include <security/SecurityManager.h>

namespace API {

class NotificationsApiService {
public:
    NotificationsApiService(PsychicHttpServer* server, SecurityManager* securityManager);

    void begin();

private:
    esp_err_t handleTelegramTest(PsychicRequest* request);

    PsychicHttpServer* _server;
    SecurityManager* _securityManager;
};

}  // namespace API
