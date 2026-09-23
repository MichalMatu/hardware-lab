#include "Application.h"

#include "../config/AppConfig.h"
#include "../config/LoggingConfig.h"
#include "../config/SensorConfig.h"
#include "../datalogger/BinaryDataLogger.h"
#include "../hardware/button/ButtonTask.h"
#include "../hardware/rtc/RTCModule.h"
#include "../hardware/rtc/RTCTask.h"

#include "../power/PowerManager.h"
#include "../sensors/SensorLoggingTask.h"
#include "FactoryReset.h"
#include "Logging.h"

using namespace POWER;

#undef LOG_TAG
#define LOG_TAG "App"

Application &Application::instance() {
  static Application app;
  return app;
}

void Application::setup(PsychicHttpServer &server, ESP32SvelteKit &framework) {
  _server = &server;
  _framework = &framework;

  LoggingConfig::begin();
  auto logCfg = LoggingConfig::get();
  LOG::Logging::begin(logCfg);
  LOGI("[Logging] level=%s ring=%u", LOG::Logging::levelToString(logCfg.level),
       logCfg.ringBufferSize);

  initPower();
  if (handleTimerWake()) {
    return;
  }

  initInputs();
  initFramework();
  _services = new ServiceRegistry(server, framework);
  _services->begin();
  initTasks();

  LOGI("Setup complete");
}

void Application::loop() {
  PowerManager::loopTick();
  delay(APP::MAIN_LOOP_DELAY_MS);
}

void Application::preSleepShutdown() {
  LOGI("[Power] Pre-sleep shutdown: stopping server and WiFi");

  // In timer-wake quick cycle we usually never start WiFi; avoid unnecessary
  // teardown work.
  wifi_mode_t mode = WiFi.getMode();
  LOGI("[Power] WiFi pre-sleep: mode=%d status=%d connected=%d apStations=%d",
       static_cast<int>(mode), static_cast<int>(WiFi.status()),
       static_cast<int>(WiFi.isConnected()),
       static_cast<int>(WiFi.softAPgetStationNum()));
  if (mode == WIFI_OFF) {
    return;
  }

  // Stop mDNS before WiFi teardown (best effort)
  MDNS.end();

  // Disconnect WiFi - this will trigger events but ESP32SvelteKit task will be
  // deleted by PowerManager after this
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void Application::onButtonEvent(ButtonEvent event) {
  PowerManager::notifyActivity();
  if (event == BTN_SHORT_PRESS) {
    LOGI("[Button] Short press - forcing sensor read and log");
    SensorLoggingTask::sendCommand(CMD_FORCE_READ_AND_LOG);
  } else if (event == BTN_LONG_PRESS) {
    LOGW("[Button] Long press - factory reset");
    System::performFactoryReset();
  }
}

void Application::initPower() {
  PowerManager::begin();
  PowerManager::setPreSleepHook(preSleepShutdown);
  _wakeReason = PowerManager::wakeReason();
  LOGI("[Power] Wake reason: %d", static_cast<int>(_wakeReason));
}

bool Application::handleTimerWake() {
  // Quick log cycle ONLY for Timer wake (no WiFi, immediate sleep)
  // All other wake reasons (Button/Unknown/Other) → full setup with WiFi
  if (_wakeReason == POWER::WakeReason::Timer) {
    SensorConfig::begin();
    DATALOG::BinaryDataLogger::begin();
    if (!RTCModule::begin()) {
      LOGW("RTC init failed; proceeding without time sync");
    } else {
      RTCModule::restoreSystemTimeOrFallback();
    }
    bool ok = SensorLoggingTask::singleShotReadAndLog();
    LOGI("Single-shot log %s", ok ? "OK" : "FAIL");
    POWER::PowerManager::requestSleep("timer-wake-cycle");
    return true;
  }
  return false;
}

void Application::initInputs() {
  ButtonTask::begin(HW::USER_BUTTON, onButtonEvent);
}

void Application::initFramework() { _framework->begin(); }

void Application::initTasks() {
  SensorConfig::begin();
  DATALOG::BinaryDataLogger::begin();

  SensorLoggingTask::begin();
  // Kick off an immediate read+log so values are visible right after boot
  SensorLoggingTask::sendCommand(CMD_FORCE_READ_AND_LOG);
  RTCTask::begin();
}
