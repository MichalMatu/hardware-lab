#include "TelegramHttpClient.h"
#include "../TelegramNotifier.h"
#include "../../system/Logging.h"
#include <ArduinoJson.h>

namespace NOTIFY {
namespace TELEGRAM {

static const char* kTelegramHost = "api.telegram.org";

String TelegramHttpClient::buildUrl(const String& botToken) {
    // Reserve exact size: https:// (8) + host (17) + /bot (4) + token + /sendMessage (12)
    const size_t exactSize = 8 + 17 + 4 + botToken.length() + 12;
    String url;
    url.reserve(exactSize + 1);  // +1 for null terminator
    url += "https://";
    url += kTelegramHost;
    url += "/bot";
    url += botToken;
    url += "/sendMessage";
    return url;
}

String TelegramHttpClient::buildPayload(const String& chatId, const String& text) {
    // Use JsonDocument with bounded allocator (avoids heap fragmentation)
    // Max payload: {"chat_id":"...","text":"..."} where text <= 1024 chars
    JsonDocument doc;
    doc["chat_id"] = chatId;
    doc["text"] = text;

    // Reserve more accurate size: {"chat_id":"...","text":"..."}
    // Overhead: {":,":""} = 20 chars + chat_id field (7) + text field (4)
    const size_t exactSize = 31 + chatId.length() + text.length();
    String payload;
    payload.reserve(exactSize + 8);  // +8 bytes safety margin for JSON escaping
    serializeJson(doc, payload);
    return payload;
}

void TelegramHttpClient::handleHttpResponse(int httpCode, HTTPClient& https, NOTIFY::TelegramSendResult& out) {
    out.httpCode = httpCode;
    LOGI("[Telegram] POST returned: httpCode=%d", httpCode);

    if (httpCode > 0) {
        // Limit response size to 512 bytes (sufficient for debugging)
        // Telegram API can return large JSON with metadata we don't need
        const int size = https.getSize();
        if (size > 0 && size <= 512) {
            out.response = https.getString();
        } else if (size > 512) {
            // Truncate large responses
            out.response = https.getString().substring(0, 512);
            out.response += "...";
        }
        return;
    }

    out.error = String("http/error: ") + HTTPClient::errorToString(httpCode);
}

bool TelegramHttpClient::sendMessage(
    const String& botToken,
    const String& chatId,
    const String& text,
    NetworkClientSecure& client,
    NOTIFY::TelegramSendResult& out
) {
    HTTPClient https;

    String url = buildUrl(botToken);
    LOGD("[Telegram] Sending to: %s", url.c_str());
    
    bool began = https.begin(client, url);
    if (!began) {
        out.error = "http/begin_failed";
        LOGW("[Telegram] HTTPClient.begin failed");
        https.end();
        return false;
    }
    
    LOGD("[Telegram] HTTPClient.begin() succeeded");

    https.addHeader("Content-Type", "application/json");

    String payload = buildPayload(chatId, text);
    LOGD("[Telegram] Sending POST: payload_len=%u", payload.length());
    
    const int httpCode = https.POST(payload);
    handleHttpResponse(httpCode, https, out);

    https.end();

    if (httpCode >= 200 && httpCode < 300) {
        return true;
    }

    if (httpCode > 0) {
        // Provide a stable, human-readable error code for callers.
        if (httpCode == 401) {
            out.error = "telegram/http_401_unauthorized";
        } else if (httpCode == 403) {
            out.error = "telegram/http_403_forbidden";
        } else if (httpCode == 404) {
            out.error = "telegram/http_404_not_found";
        } else if (httpCode == 429) {
            out.error = "telegram/http_429_rate_limited";
        } else if (httpCode >= 400 && httpCode < 500) {
            out.error = String("telegram/http_4xx_") + httpCode;
        } else if (httpCode >= 500) {
            out.error = String("telegram/http_5xx_") + httpCode;
        } else {
            out.error = String("telegram/http_") + httpCode;
        }
        return false;
    }

    char tlsErr[192] = {0};
    client.lastError(tlsErr, sizeof(tlsErr));
    if (tlsErr[0] != '\0') {
        out.tlsError = tlsErr;
    }

    LOGW("[Telegram] Send failed: code=%d err=%s tls=%s", httpCode, out.error.c_str(), out.tlsError.c_str());
    return false;
}

}  // namespace TELEGRAM
}  // namespace NOTIFY
