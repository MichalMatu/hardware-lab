#pragma once

#include <PsychicHttpServer.h>
#include <core/ESP32SvelteKit.h>

namespace API {
class SensorsApiService;
class LogsApiService;
class ChartsApiService;
class ConfigApiService;
class RtcApiService;
class NotificationsApiService;
class Rf433ApiService;
} // namespace API

class PowerApiService;
class TelegramSettingsService;
class Rf433Manager;
class Rf433SettingsService;

class ServiceRegistry {
public:
  ServiceRegistry(PsychicHttpServer &server, ESP32SvelteKit &framework);
  ~ServiceRegistry();

  void begin();

  TelegramSettingsService *getTelegramSettingsService() const {
    return _telegramSettings;
  }

private:
  PsychicHttpServer *_server;
  ESP32SvelteKit *_framework;

  PowerApiService *_powerApi{nullptr};
  API::SensorsApiService *_sensorsApi{nullptr};
  API::LogsApiService *_logsApi{nullptr};
  API::ChartsApiService *_chartsApi{nullptr};
  API::ConfigApiService *_configApi{nullptr};
  API::RtcApiService *_rtcApi{nullptr};
  API::NotificationsApiService *_notificationsApi{nullptr};
  TelegramSettingsService *_telegramSettings{nullptr};
  
  // RF433 integration
  Rf433Manager *_rf433Manager{nullptr};
  Rf433SettingsService *_rf433Settings{nullptr};
  API::Rf433ApiService *_rf433Api{nullptr};
};
