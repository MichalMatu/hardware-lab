#pragma once

#include <Arduino.h>

#include "AppTypes.h"
#include "CameraService.h"
#include "ConfigManager.h"
#include "MotionDetector.h"
#include "NetworkService.h"
#include "StorageService.h"
#include "TelegramClient.h"
#include "WebPanel.h"

class AppController {
 public:
  void begin();
  void loop();
  bool capturePhoto(PhotoReason reason, bool sendTelegram);
  void reboot();

 private:
  static bool captureThunk(void* context, PhotoReason reason, bool sendTelegram);
  static void rebootThunk(void* context);
  static void armStateThunk(void* context, ArmState state);
  static void webTaskThunk(void* context);
  static void telegramTaskThunk(void* context);
  static void motionTaskThunk(void* context);

  void webTask();
  void telegramTask();
  void motionTask();
  void setArmState(ArmState state);
  void handleTelegramCommand(const TelegramCommand& command);
  void sendStatus();
  bool pollTelegram(TelegramCommand* commands, size_t maxCommands, size_t& count);
  bool sendTelegramMessage(const char* text);
  bool sendTelegramPhoto(const uint8_t* jpeg, size_t length, const char* caption);
  bool sampleMotion(bool& motionDetected);
  void restoreFlashOutput();
  bool shouldUseFlashLocked(PhotoReason reason);
  bool measureBrightnessLocked(uint8_t& brightness);
  bool telegramConfigured() const;
  const char* reasonName(PhotoReason reason) const;

  AppConfig config_;
  AppState state_;
  ConfigManager configManager_;
  CameraService camera_;
  StorageService storage_;
  NetworkService network_;
  TelegramClient telegram_;
  MotionDetector motion_;
  WebPanel web_;

  SemaphoreHandle_t cameraMutex_ = nullptr;
  SemaphoreHandle_t telegramMutex_ = nullptr;
  TaskHandle_t webTaskHandle_ = nullptr;
  TaskHandle_t telegramTaskHandle_ = nullptr;
  TaskHandle_t motionTaskHandle_ = nullptr;
};
