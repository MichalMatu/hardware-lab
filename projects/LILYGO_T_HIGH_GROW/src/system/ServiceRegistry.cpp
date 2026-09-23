#include "ServiceRegistry.h"

#include "../api/charts/ChartsApiService.h"
#include "../api/config/ConfigApiService.h"
#include "../api/logs/LogsApiService.h"
#include "../api/notifications/NotificationsApiService.h"
#include "../api/power/PowerApiService.h"
#include "../api/rf433/Rf433ApiService.h"
#include "../api/rtc/RtcApiService.h"
#include "../api/sensors/SensorsApiService.h"
#include "../hardware/rf433/devices/Rf433Manager.h"
#include "../hardware/rf433/core/Rf433RadioController.h"
#include "../hardware/rf433/settings/Rf433SettingsService.h"
#include "../notifications/telegram/TelegramSettingsService.h"

#include "Logging.h"

#undef LOG_TAG
#define LOG_TAG "ServiceRegistry"

ServiceRegistry::ServiceRegistry(PsychicHttpServer &server,
                                 ESP32SvelteKit &framework)
    : _server(&server), _framework(&framework) {}

ServiceRegistry::~ServiceRegistry() {
  // Optional: cleanup if needed, though objects are usually long-lived
}

void ServiceRegistry::begin() {
  LOGI("Initializing services...");

  // Initialize RF433 radio controller task
  Rf433RadioController::init();

  _telegramSettings = new TelegramSettingsService(
      _server, _framework->getFS(), _framework->getSecurityManager());
  _telegramSettings->begin();

  _powerApi = new PowerApiService(_server, _framework->getSecurityManager());
  _powerApi->begin();

  _sensorsApi =
      new API::SensorsApiService(_server, _framework->getSecurityManager());
  _sensorsApi->begin();

  _logsApi = new API::LogsApiService(_server, _framework->getSecurityManager());
  _logsApi->begin();

  _chartsApi =
      new API::ChartsApiService(_server, _framework->getSecurityManager());
  _chartsApi->begin();

  _configApi =
      new API::ConfigApiService(_server, _framework->getSecurityManager());
  _configApi->begin();

  _rtcApi = new API::RtcApiService(_server, _framework->getSecurityManager());
  _rtcApi->begin();

  _notificationsApi = new API::NotificationsApiService(
      _server, _framework->getSecurityManager());
  _notificationsApi->begin();

  // RF433 integration
  _rf433Manager = new Rf433Manager();
  _rf433Settings = new Rf433SettingsService(
      _server, _framework->getFS(), _framework->getSecurityManager(), _rf433Manager);
  _rf433Settings->begin();
  
  _rf433Api = new API::Rf433ApiService(
      _server, _framework->getSecurityManager(), _rf433Manager, _rf433Settings);
  _rf433Api->begin();

  LOGI("All services initialized");
}
