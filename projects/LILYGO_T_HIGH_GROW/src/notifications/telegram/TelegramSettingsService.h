#pragma once

#include <core/StatefulService.h>
#include <core/HttpEndpoint.h>
#include <core/FSPersistence.h>
#include <utils/SettingValue.h>

#ifndef FACTORY_TELEGRAM_ENABLED
#define FACTORY_TELEGRAM_ENABLED false
#endif

#ifndef FACTORY_TELEGRAM_BOT_TOKEN
#define FACTORY_TELEGRAM_BOT_TOKEN ""
#endif

#ifndef FACTORY_TELEGRAM_CHAT_ID
#define FACTORY_TELEGRAM_CHAT_ID ""
#endif

#define TELEGRAM_SETTINGS_FILE "/config/telegramSettings.json"
#define TELEGRAM_SETTINGS_SERVICE_PATH "/rest/telegramSettings"

class TelegramSettings {
public:
    bool enabled;
    String botToken;
    String chatId;

    static void read(TelegramSettings& settings, JsonObject& root) {
        root["enabled"] = settings.enabled;
        root["bot_token"] = settings.botToken;
        root["chat_id"] = settings.chatId;
    }

    static StateUpdateResult update(JsonObject& root, TelegramSettings& settings, const String& originId) {
        settings.enabled = root["enabled"] | FACTORY_TELEGRAM_ENABLED;
        settings.botToken = root["bot_token"] | FACTORY_TELEGRAM_BOT_TOKEN;
        settings.chatId = root["chat_id"] | FACTORY_TELEGRAM_CHAT_ID;
        return StateUpdateResult::CHANGED;
    }
};

class TelegramSettingsService : public StatefulService<TelegramSettings> {
public:
    TelegramSettingsService(PsychicHttpServer* server, FS* fs, SecurityManager* securityManager);

    void begin();

    bool isEnabled() const;
    bool isConfigured() const;
    String getBotToken() const;
    String getChatId() const;

private:
    HttpEndpoint<TelegramSettings> _httpEndpoint;
    FSPersistence<TelegramSettings> _fsPersistence;

    void onConfigUpdated();
};
