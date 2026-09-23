#include "TelegramSettingsService.h"
#include "../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "TelegramSettings"

TelegramSettingsService::TelegramSettingsService(PsychicHttpServer* server, FS* fs, SecurityManager* securityManager)
    : _httpEndpoint(TelegramSettings::read,
                    TelegramSettings::update,
                    this,
                    server,
                    TELEGRAM_SETTINGS_SERVICE_PATH,
                    securityManager,
                    AuthenticationPredicates::IS_AUTHENTICATED),
      _fsPersistence(TelegramSettings::read, TelegramSettings::update, this, fs, TELEGRAM_SETTINGS_FILE) {
    
    addUpdateHandler([&](const String& originId) { onConfigUpdated(); }, false);
}

void TelegramSettingsService::begin() {
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
    
    const TelegramSettings& settings = _state;
    LOGI("Telegram settings loaded: enabled=%d, bot_token_len=%u, chat_id_len=%u",
         settings.enabled ? 1 : 0,
         settings.botToken.length(),
         settings.chatId.length());
}

void TelegramSettingsService::onConfigUpdated() {
    const TelegramSettings& settings = _state;
    LOGI("Telegram settings updated: enabled=%d, bot_token_len=%u, chat_id_len=%u",
         settings.enabled ? 1 : 0,
         settings.botToken.length(),
         settings.chatId.length());
}

bool TelegramSettingsService::isEnabled() const {
    return _state.enabled;
}

bool TelegramSettingsService::isConfigured() const {
    return _state.botToken.length() > 0 && _state.chatId.length() > 0;
}

String TelegramSettingsService::getBotToken() const {
    return _state.botToken;
}

String TelegramSettingsService::getChatId() const {
    return _state.chatId;
}
