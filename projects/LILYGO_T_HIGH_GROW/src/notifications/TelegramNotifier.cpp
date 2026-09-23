#include "TelegramNotifier.h"
#include "telegram/TelegramConnectionValidator.h"
#include "telegram/TelegramHttpClient.h"
#include "telegram/TelegramSettingsService.h"
#include "telegram/TelegramTlsConfig.h"

#include "../config/AppConfig.h"
#include "../system/Logging.h"

#include <NetworkClientSecure.h>

namespace NOTIFY {

void TelegramNotifier::setSettingsService(
    TelegramSettingsService *settingsService) {
  _settingsService = settingsService;
}

bool TelegramNotifier::isConfigured() const {
  if (!_settingsService) {
    LOGW("[Telegram] Settings service not initialized");
    return false;
  }

  const bool configured = _settingsService->isConfigured();
  LOGI("[Telegram] isConfigured: %s", configured ? "YES" : "NO");
  return configured;
}

bool TelegramNotifier::sendMessage(const String &text,
                                   TelegramSendResult &out) {
  out = TelegramSendResult{};

  if (!_settingsService) {
    out.error = "Telegram settings service not initialized";
    LOGE("[Telegram] Settings service not initialized");
    return false;
  }

  if (!_settingsService->isEnabled()) {
    out.error = "Telegram notifications disabled";
    LOGI("[Telegram] Notifications disabled in settings");
    return false;
  }

  if (!isConfigured()) {
    out.error = "Telegram not configured (bot_token/chat_id missing)";
    return false;
  }

  if (text.length() == 0) {
    out.error = "input/missing_text";
    return false;
  }

  if (text.length() > APP::NOTIFY::TELEGRAM_MAX_TEXT_LEN) {
    out.error = "input/text_too_long";
    return false;
  }

  LOGI("[Telegram] Starting send: text_len=%u", text.length());

  // Hard policy: do not attempt Telegram unless WiFi is connected AND SNTP has
  // synced. This is used as an internet reachability check.
  String onlineErr;
  if (!TELEGRAM::TelegramConnectionValidator::ensureOnline(
          onlineErr, APP::NOTIFY::TELEGRAM_TIME_WAIT_MS)) {
    out.error = onlineErr;
    LOGW("[Telegram] Offline (policy): %s", onlineErr.c_str());
    return false;
  }

  LOGI("[Telegram] HEAP: free=%u maxAlloc=%u", ESP.getFreeHeap(),
       ESP.getMaxAllocHeap());

  // Declare client before HTTPClient so destruct order is safe.
  NetworkClientSecure client;
  LOGD("[Telegram] NetworkClientSecure created");

  TELEGRAM::TelegramTlsConfig::configure(client);

  const String botToken = _settingsService->getBotToken();
  const String chatId = _settingsService->getChatId();

  const bool success = TELEGRAM::TelegramHttpClient::sendMessage(
      botToken, chatId, text, client, out);

  client.stop();

  if (!success) {
    LOGW("[Telegram] Send failed: code=%d err=%s tls=%s", out.httpCode,
         out.error.c_str(), out.tlsError.c_str());
  }

  return success;
}

} // namespace NOTIFY
