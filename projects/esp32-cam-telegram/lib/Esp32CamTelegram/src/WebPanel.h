#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include "AppTypes.h"
#include "ConfigManager.h"

class WebPanel {
 public:
  using CaptureCallback = bool (*)(void* context, PhotoReason reason, bool sendTelegram);
  using RebootCallback = void (*)(void* context);
  using ArmStateCallback = void (*)(void* context, ArmState state);

  void begin(AppConfig& config, AppState& state, ConfigManager& configManager, CaptureCallback capture, RebootCallback reboot, ArmStateCallback armState, void* context);
  void handle();

 private:
  bool authorize();
  void root();
  void statusJson();
  void save();
  void capture();
  void arm();
  void disarm();
  void privacy();
  void resetConfig();
  void reboot();
  void lastPhoto();
  void copyArg(const char* name, char* dest, size_t destLen);
  void sendChunk(const char* text);
  void sendEscaped(const char* text);
  void notFound();

  WebServer server_{80};
  AppConfig* config_ = nullptr;
  AppState* state_ = nullptr;
  ConfigManager* configManager_ = nullptr;
  CaptureCallback capture_ = nullptr;
  RebootCallback reboot_ = nullptr;
  ArmStateCallback armState_ = nullptr;
  void* context_ = nullptr;
};
