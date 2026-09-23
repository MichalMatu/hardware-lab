#include "TelegramChatIdFetcher.h"
#include "../../system/Logging.h"
#include <ArduinoJson.h>

namespace NOTIFY {
namespace TELEGRAM {

String TelegramChatIdFetcher::buildGetUpdatesUrl(const String& botToken) {
    // Reserve: https:// (8) + host (17) + /bot (4) + token + /getUpdates?limit=10 (20)
    const size_t exactSize = 8 + 17 + 4 + botToken.length() + 20;
    String url;
    url.reserve(exactSize + 1);
    url += "https://";
    url += kTelegramHost;
    url += "/bot";
    url += botToken;
    url += "/getUpdates?limit=";
    url += String(kUpdateLimit);
    return url;
}

void TelegramChatIdFetcher::extractChatsFromJson(
    const String& response,
    TelegramGetUpdatesResult& out
) {
    LOGD("[TelegramChatId] Parsing JSON response (len=%u)", response.length());
    LOGD("[TelegramChatId] JSON payload: %s", response.c_str());

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, response);
    if (err) {
        out.error = String("json/parse_error: ") + err.c_str();
        LOGW("[TelegramChatId] JSON parse error: %s", err.c_str());
        return;
    }

    const bool ok = doc["ok"].as<bool>();
    LOGD("[TelegramChatId] Parsed JSON: ok=%d", ok);
    if (!ok) {
        out.error = "telegram/api_error";
        const char* description = doc["description"].as<const char*>();
        if (description) {
            out.error += ": ";
            out.error += description;
        }
        LOGW("[TelegramChatId] API returned ok=false");
        return;
    }

    JsonArray result = doc["result"].as<JsonArray>();
    if (result.isNull()) {
        LOGI("[TelegramChatId] No updates found (result array is null)");
        return;
    }

    const size_t resultSize = result.size();
    LOGI("[TelegramChatId] Found %u updates in result array", resultSize);
    if (resultSize == 0) {
        LOGI("[TelegramChatId] No updates found (result array is empty)");
        return;
    }

    // Extract unique chats (avoid duplicates from multiple messages)
    std::vector<String> seenChatIds;
    int updateIndex = 0;
    for (JsonVariant update : result) {
        LOGD("[TelegramChatId] Processing update[%d]", updateIndex++);

        JsonObject message = update["message"].as<JsonObject>();
        if (message.isNull()) {
            LOGD("[TelegramChatId] Update has no 'message' field, skipping");
            continue;
        }

        JsonObject chat = message["chat"].as<JsonObject>();
        if (chat.isNull()) {
            LOGW("[TelegramChatId] Message has no 'chat' field, skipping");
            continue;
        }

        // Extract chat ID (can be int64, so read as string)
        String chatId;
        if (chat["id"].is<long long>()) {
            chatId = String(chat["id"].as<long long>());
        } else {
            chatId = String(chat["id"].as<long>());
        }

        // Skip duplicates
        bool isDuplicate = false;
        for (const String& seen : seenChatIds) {
            if (seen == chatId) {
                isDuplicate = true;
                break;
            }
        }
        if (isDuplicate) {
            continue;
        }
        seenChatIds.push_back(chatId);

        TelegramChat telegramChat;
        telegramChat.id = chatId;
        telegramChat.type = chat["type"].as<const char*>();
        telegramChat.firstName = chat["first_name"].as<const char*>();
        telegramChat.lastName = chat["last_name"].as<const char*>();
        telegramChat.username = chat["username"].as<const char*>();
        telegramChat.title = chat["title"].as<const char*>();

        out.chats.push_back(telegramChat);
        LOGI("[TelegramChatId] Found chat: id=%s type=%s", 
             chatId.c_str(), telegramChat.type.c_str());
    }

    LOGI("[TelegramChatId] Extracted %u unique chats", out.chats.size());
}

void TelegramChatIdFetcher::parseGetUpdatesResponse(
    int httpCode,
    HTTPClient& https,
    TelegramGetUpdatesResult& out
) {
    out.httpCode = httpCode;
    LOGI("[TelegramChatId] GET getUpdates returned: httpCode=%d", httpCode);

    if (httpCode >= 200 && httpCode < 300) {
        const int contentLength = https.getSize();
        if (contentLength > 0 && contentLength > static_cast<int>(kMaxResponseBytes)) {
            out.error = "telegram/response_too_large";
            LOGW("[TelegramChatId] Response too large: %d bytes (limit %u)", contentLength, kMaxResponseBytes);
            return;
        }

        const String response = https.getString();
        if (response.length() > kMaxResponseBytes) {
            out.error = "telegram/response_too_large";
            LOGW("[TelegramChatId] Response exceeded limit after read: %u bytes (limit %u)", response.length(), kMaxResponseBytes);
            return;
        }

        LOGD("[TelegramChatId] Response size: %u bytes", response.length());
        extractChatsFromJson(response, out);
        return;
    }

    // HTTP error
    if (httpCode > 0) {
        if (httpCode == 401) {
            out.error = "telegram/http_401_unauthorized";
        } else if (httpCode == 404) {
            out.error = "telegram/http_404_not_found";
        } else if (httpCode >= 400 && httpCode < 500) {
            out.error = String("telegram/http_4xx_") + httpCode;
        } else if (httpCode >= 500) {
            out.error = String("telegram/http_5xx_") + httpCode;
        } else {
            out.error = String("telegram/http_") + httpCode;
        }
        return;
    }

    // Connection error
    out.error = String("http/error: ") + HTTPClient::errorToString(httpCode);
}

bool TelegramChatIdFetcher::getUpdates(
    const String& botToken,
    NetworkClientSecure& client,
    TelegramGetUpdatesResult& out
) {
    HTTPClient https;

    String url = buildGetUpdatesUrl(botToken);
    LOGD("[TelegramChatId] Getting updates from: %s", url.c_str());
    
    bool began = https.begin(client, url);
    if (!began) {
        out.error = "http/begin_failed";
        LOGW("[TelegramChatId] HTTPClient.begin failed");
        https.end();
        return false;
    }
    
    // Set 10 second timeout
    https.setTimeout(kTimeoutMs);
    LOGD("[TelegramChatId] HTTPClient.begin() succeeded, timeout set to %dms", 
         kTimeoutMs);

    const int httpCode = https.GET();
    parseGetUpdatesResponse(httpCode, https, out);

    https.end();

    if (httpCode >= 200 && httpCode < 300) {
        return true;
    }

    char tlsErr[192] = {0};
    client.lastError(tlsErr, sizeof(tlsErr));
    if (tlsErr[0] != '\0') {
        out.tlsError = tlsErr;
    }

    LOGW("[TelegramChatId] getUpdates failed: code=%d err=%s tls=%s", 
         httpCode, out.error.c_str(), out.tlsError.c_str());
    return false;
}

bool TelegramChatIdFetcher::getBotInfo(
    const String& botToken,
    NetworkClientSecure& client
) {
    HTTPClient https;
    String url = "https://" + String(kTelegramHost) + "/bot" + botToken + "/getMe";
    
    LOGI("[TelegramChatId] Checking bot info: %s", url.c_str());
    
    if (!https.begin(client, url)) {
        LOGW("[TelegramChatId] getMe failed to begin");
        return false;
    }
    
    https.setTimeout(kTimeoutMs);
    int httpCode = https.GET();
    
    if (httpCode == 200) {
        String payload = https.getString();
        LOGI("[TelegramChatId] getMe response: %s", payload.c_str());
        
        JsonDocument doc;
        deserializeJson(doc, payload);
        const char* username = doc["result"]["username"] | "unknown";
        const char* first_name = doc["result"]["first_name"] | "unknown";
        LOGI("[TelegramChatId] Connected as @%s (%s)", username, first_name);
        https.end();
        return true;
    }
    
    LOGW("[TelegramChatId] getMe failed: %d", httpCode);
    https.end();
    return false;
}

}  // namespace TELEGRAM
}  // namespace NOTIFY
