#include "NotificationsApiService.h"
#include "TelegramTestTask.h"
#include "GetChatIdHandler.h"

#include "../../config/AppConfig.h"
#include "../../power/PowerManager.h"
#include "../../system/Application.h"
#include "../../system/ErrorCodes.h"
#include "../../system/Logging.h"

#include <ArduinoJson.h>
#include <PsychicJson.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace API {

namespace {
constexpr size_t kMaxTelegramTextLen = APP::NOTIFY::TELEGRAM_MAX_TEXT_LEN;

// Prevent concurrent TLS handshakes in the test endpoint.
// NOTE: We intentionally use a binary semaphore (NOT a mutex). A FreeRTOS mutex
// must be released by the same task that acquired it (ownership + priority
// inheritance). Our test send runs in a separate task, so releasing a mutex
// from that task can trigger an assert (xTaskPriorityDisinherit). A binary
// semaphore is safe here.
SemaphoreHandle_t gTelegramTestSemaphore = nullptr;
} // namespace

NotificationsApiService::NotificationsApiService(
    PsychicHttpServer *server, SecurityManager *securityManager)
    : _server(server), _securityManager(securityManager) {}

void NotificationsApiService::begin() {
  auto adminWrap = [this](auto fn) {
    return _securityManager->wrapRequest(
        [fn](PsychicRequest *request) -> esp_err_t {
          POWER::PowerManager::notifyActivity("api/notifications");
          return fn(request);
        },
        AuthenticationPredicates::IS_ADMIN);
  };

  _server->on("/api/notifications/telegram/test", HTTP_POST,
              adminWrap([this](PsychicRequest *request) {
                return handleTelegramTest(request);
              }));

  // Register GetChatId handler
  GetChatIdHandler::attachHandler(_server, _securityManager);

  if (gTelegramTestSemaphore == nullptr) {
    gTelegramTestSemaphore = xSemaphoreCreateBinary();
    if (gTelegramTestSemaphore == nullptr) {
      LOGW("[TelegramAPI] Failed to create test semaphore; concurrent tests "
           "will be allowed");
    } else {
      // Mark semaphore as available.
      xSemaphoreGive(gTelegramTestSemaphore);
    }
  }
}

esp_err_t NotificationsApiService::handleTelegramTest(PsychicRequest *request) {
  LOGI("[TelegramAPI] Handler started");

  TelegramSettingsService *telegramSettings =
      Application::instance().getTelegramSettingsService();
  if (!telegramSettings) {
    PsychicJsonResponse response(request);
    JsonVariant &root = response.getRoot();
    root["ok"] = false;
    root["configured"] = false;
    root["error"] = ErrorCodes::Service::TELEGRAM_SETTINGS_UNAVAILABLE;
    return response.send();
  }

  const bool configured = telegramSettings->isConfigured();
  LOGI("[TelegramAPI] isConfigured=%d", configured ? 1 : 0);

  // Always return JSON so callers have a stable contract.
  if (!configured) {
    PsychicJsonResponse response(request);
    JsonVariant &root = response.getRoot();
    root["ok"] = false;
    root["configured"] = false;
    root["error"] = ErrorCodes::Config::NOT_CONFIGURED;
    return response.send();
  }

  const String body = request->body();
  LOGI("[TelegramAPI] Body received, len=%u", body.length());

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  LOGI("[TelegramAPI] deserializeJson result: %d", err.code());

  if (err) {
    PsychicJsonResponse response(request);
    JsonVariant &root = response.getRoot();
    root["ok"] = false;
    root["configured"] = true;
    root["error"] = String(ErrorCodes::Input::JSON_PARSE_ERROR) + ": " + err.c_str();
    return response.send();
  }

  // Accept both "text" (canonical) and legacy "message" from older UI.
  const char *text = doc["text"].as<const char *>();
  if (!text || text[0] == '\0') {
    text = doc["message"].as<const char *>();
  }
  if (!text || text[0] == '\0') {
    PsychicJsonResponse response(request);
    JsonVariant &root = response.getRoot();
    root["ok"] = false;
    root["configured"] = true;
    root["error"] = ErrorCodes::Input::EMPTY_TEXT;
    return response.send();
  }

  if (strlen(text) > kMaxTelegramTextLen) {
    PsychicJsonResponse response(request);
    JsonVariant &root = response.getRoot();
    root["ok"] = false;
    root["configured"] = true;
    root["error"] = ErrorCodes::Input::TEXT_TOO_LONG;
    return response.send();
  }

  if (gTelegramTestSemaphore != nullptr) {
    if (xSemaphoreTake(gTelegramTestSemaphore, 0) != pdTRUE) {
      PsychicJsonResponse response(request);
      response.setCode(429);
      JsonVariant &root = response.getRoot();
      root["ok"] = false;
      root["configured"] = true;
      root["error"] = ErrorCodes::Busy::TELEGRAM_TEST_IN_PROGRESS;
      return response.send();
    }
  }

  LOGI("[TelegramAPI] Queuing Telegram send on worker");

  NOTIFY::TelegramSendResult result;
  const bool completed = TelegramTest::submitAndWait(
      text,
      telegramSettings,
      gTelegramTestSemaphore,
      30000,
      result);

  if (!completed) {
    // Handler timed out or worker not available. The worker (if running)
    // will still finish and release the concurrency semaphore.
    PsychicJsonResponse response(request);
    JsonVariant &root = response.getRoot();
    root["ok"] = false;
    root["configured"] = true;
    root["error"] = ErrorCodes::Internal::TASK_TIMEOUT;
    return response.send();
  }

  const bool ok = (result.httpCode >= 200 && result.httpCode < 300);
  LOGI("[TelegramAPI] Task completed: ok=%d httpCode=%d", ok, result.httpCode);

  // Return JSON response
  PsychicJsonResponse response(request);
  JsonVariant &root = response.getRoot();
  root["ok"] = ok;
  root["configured"] = true;
  root["httpCode"] = result.httpCode;
  root["error"] = result.error;
  root["tlsError"] = result.tlsError;
  root["response"] = result.response;

  return response.send();
}

} // namespace API