#pragma once

#include "../hardware/button/ButtonTask.h"
#include "../power/PowerManager.h"
#include <PsychicHttpServer.h>
#include <core/ESP32SvelteKit.h>

// Services moved to ServiceRegistry

#include "ServiceRegistry.h"

class Application {
public:
  static Application &instance();

  void setup(PsychicHttpServer &server, ESP32SvelteKit &framework);
  void loop();

  TelegramSettingsService *getTelegramSettingsService() {
    return _services->getTelegramSettingsService();
  }

private:
  Application() = default;
  static void preSleepShutdown();
  static void onButtonEvent(ButtonEvent event);

  void initPower();
  bool handleTimerWake();
  void initInputs();
  void initFramework();

  void initTasks();

  PsychicHttpServer *_server{nullptr};
  ESP32SvelteKit *_framework{nullptr};

  // Services
  ServiceRegistry *_services{nullptr};

  POWER::WakeReason _wakeReason{POWER::WakeReason::Other};
};
