/**
 * @file TelegramChatIdFetcher.h
 * @brief Fetches available chat IDs from Telegram Bot API
 *
 * Provides functionality to retrieve recent chat updates from Telegram
 * using the getUpdates endpoint. Used for discovering chat IDs after
 * a user sends their first message to the bot.
 */

#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <vector>

namespace NOTIFY {
namespace TELEGRAM {

/**
 * Represents a Telegram chat extracted from getUpdates response
 */
struct TelegramChat {
    String id;           // Chat ID (can be negative for groups)
    String type;         // "private", "group", "supergroup", "channel"
    String firstName;    // First name (for private chats)
    String lastName;     // Last name (for private chats)
    String username;     // Username (optional)
    String title;        // Title (for groups/channels)
};

/**
 * Result of getUpdates operation
 */
struct TelegramGetUpdatesResult {
    int httpCode{0};
    String error;
    String tlsError;
    std::vector<TelegramChat> chats;
};

/**
 * Fetches chat IDs from Telegram Bot API
 */
class TelegramChatIdFetcher {
public:
    /**
     * Fetch recent chat updates and extract unique chat IDs
     * 
     * @param botToken Telegram bot token
     * @param client Configured TLS client
     * @param out Result structure to fill
     * @return true if request succeeded (HTTP 2xx), false otherwise
     */
    static bool getUpdates(
        const String& botToken,
        NetworkClientSecure& client,
        TelegramGetUpdatesResult& out
    );

    /**
     * Verifies bot token and logs bot details (getMe)
     */
    static bool getBotInfo(
        const String& botToken,
        NetworkClientSecure& client
    );

private:
    static String buildGetUpdatesUrl(const String& botToken);
    static void parseGetUpdatesResponse(
        int httpCode,
        HTTPClient& https,
        TelegramGetUpdatesResult& out
    );
    static void extractChatsFromJson(
        const String& response,
        TelegramGetUpdatesResult& out
    );

    static constexpr const char* kTelegramHost = "api.telegram.org";
    static constexpr int kTimeoutMs = 10000;  // 10 second timeout
    static constexpr int kUpdateLimit = 10;   // Limit updates to 10 most recent
    static constexpr size_t kMaxResponseBytes = 4096; // Cap response to avoid RAM blowup
};

}  // namespace TELEGRAM
}  // namespace NOTIFY
