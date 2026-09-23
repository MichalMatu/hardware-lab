#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>

namespace NOTIFY {

struct TelegramSendResult;

namespace TELEGRAM {

class TelegramHttpClient {
public:
    static bool sendMessage(
        const String& botToken,
        const String& chatId,
        const String& text,
        NetworkClientSecure& client,
        NOTIFY::TelegramSendResult& out
    );

private:
    static String buildUrl(const String& botToken);
    static String buildPayload(const String& chatId, const String& text);
    static void handleHttpResponse(int httpCode, HTTPClient& https, NOTIFY::TelegramSendResult& out);
};

}  // namespace TELEGRAM
}  // namespace NOTIFY
