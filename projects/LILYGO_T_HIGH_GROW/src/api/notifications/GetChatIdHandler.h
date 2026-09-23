/**
 * @file GetChatIdHandler.h
 * @brief API handler for fetching Telegram chat IDs
 *
 * Provides HTTP endpoint to fetch available chat IDs from Telegram Bot API.
 * Endpoint: POST /api/notifications/telegram/get-chat-id
 */

#pragma once

#include <PsychicHttpServer.h>
#include <security/SecurityManager.h>

namespace API {

/**
 * Handler for Telegram chat ID discovery endpoint
 */
class GetChatIdHandler {
public:
    /**
     * Attach handler to server
     * 
     * @param server HTTP server instance
     * @param securityManager Security manager for authentication
     */
    static void attachHandler(
        PsychicHttpServer* server,
        SecurityManager* securityManager
    );

private:
    static esp_err_t handleRequest(PsychicRequest* request);
};

}  // namespace API
