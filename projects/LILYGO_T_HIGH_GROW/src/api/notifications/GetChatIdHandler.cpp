#include "GetChatIdHandler.h"
#include "../../config/AppConfig.h"
#include "../../notifications/telegram/TelegramChatIdFetcher.h"
#include "../../notifications/telegram/TelegramTlsConfig.h"
#include "../../power/PowerManager.h"
#include "../../system/ErrorCodes.h"
#include "../../system/Logging.h"

#include <ArduinoJson.h>
#include <PsychicJson.h>

namespace API {

namespace {
bool isBotTokenFormatValid(const char* token) {
    if (!token) return false;

    const size_t len = strlen(token);
    if (len < 10) return false;

    // Expected format: <digits>:<alnum/_->
    const char* colon = strchr(token, ':');
    if (!colon) return false;
    if (colon == token) return false;  // nothing before colon

    // Validate left side (digits)
    for (const char* p = token; p < colon; ++p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
    }

    // Validate right side (length >= 10, allowed A-Z a-z 0-9 _ -)
    const char* right = colon + 1;
    size_t rightLen = strlen(right);
    if (rightLen < 10) return false;
    for (const char* p = right; *p; ++p) {
        const char c = *p;
        const bool isAlphaNum = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        if (isAlphaNum || c == '_' || c == '-') continue;
        return false;
    }

    return true;
}
}  // namespace

void GetChatIdHandler::attachHandler(
    PsychicHttpServer* server,
    SecurityManager* securityManager
) {
    auto adminWrap = [securityManager](auto fn) {
        return securityManager->wrapRequest(
            [fn](PsychicRequest* request) -> esp_err_t {
                POWER::PowerManager::notifyActivity("api/telegram/get-chat-id");
                return fn(request);
            },
            AuthenticationPredicates::IS_ADMIN);
    };

    server->on("/api/notifications/telegram/get-chat-id", HTTP_POST,
               adminWrap([](PsychicRequest* request) {
                   return handleRequest(request);
               }));

    LOGI("[GetChatIdHandler] Endpoint registered: POST /api/notifications/telegram/get-chat-id");
}

esp_err_t GetChatIdHandler::handleRequest(PsychicRequest* request) {
    LOGI("[GetChatIdHandler] Request started");

    const String body = request->body();
    LOGI("[GetChatIdHandler] Body received, len=%u", body.length());

    // Parse request body
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);

    if (err) {
        PsychicJsonResponse response(request);
        JsonVariant& root = response.getRoot();
        root["ok"] = false;
        root["error"] = String(ErrorCodes::Input::JSON_PARSE_ERROR) + ": " + err.c_str();
        return response.send();
    }

    // Validate bot token
    const char* botToken = doc["bot_token"].as<const char*>();
    if (!isBotTokenFormatValid(botToken)) {
        PsychicJsonResponse response(request);
        JsonVariant& root = response.getRoot();
        root["ok"] = false;
        root["error"] = ErrorCodes::Input::INVALID_FORMAT;
        LOGW("[GetChatIdHandler] Invalid bot_token format");
        return response.send();
    }

    LOGI("[GetChatIdHandler] Calling getUpdates with bot token (len=%u)", strlen(botToken));

    // Initialize TLS client
    NetworkClientSecure client;
    NOTIFY::TELEGRAM::TelegramTlsConfig::configure(client);

    // Debug: Verify bot identity first
    NOTIFY::TELEGRAM::TelegramChatIdFetcher::getBotInfo(botToken, client);

    // Fetch updates
    NOTIFY::TELEGRAM::TelegramGetUpdatesResult result;
    const bool success = NOTIFY::TELEGRAM::TelegramChatIdFetcher::getUpdates(
        botToken,
        client,
        result
    );

    client.stop();

    LOGI("[GetChatIdHandler] getUpdates completed: success=%d httpCode=%d chats=%u",
         success, result.httpCode, result.chats.size());

    // Build response
    PsychicJsonResponse response(request);
    JsonVariant& root = response.getRoot();
    root["ok"] = success;
    root["httpCode"] = result.httpCode;

    if (!success) {
        root["error"] = result.error;
        root["tlsError"] = result.tlsError;
        return response.send();
    }

    // Add chats array
    JsonArray chatsArray = root["chats"].to<JsonArray>();
    for (const NOTIFY::TELEGRAM::TelegramChat& chat : result.chats) {
        JsonObject chatObj = chatsArray.add<JsonObject>();
        chatObj["id"] = chat.id;
        chatObj["type"] = chat.type;
        
        // Build display name
        String name;
        if (!chat.title.isEmpty()) {
            name = chat.title;  // Groups/channels
        } else {
            if (!chat.firstName.isEmpty()) {
                name = chat.firstName;
                if (!chat.lastName.isEmpty()) {
                    name += " ";
                    name += chat.lastName;
                }
            }
            if (!chat.username.isEmpty()) {
                if (!name.isEmpty()) {
                    name += " (@";
                    name += chat.username;
                    name += ")";
                } else {
                    name = "@";
                    name += chat.username;
                }
            }
        }
        
        if (!name.isEmpty()) {
            chatObj["name"] = name;
        }
    }

    return response.send();
}

}  // namespace API
