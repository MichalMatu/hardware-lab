#pragma once

#include <Arduino.h>

enum class ArmState : uint8_t {
  Disarmed,
  Armed,
  Privacy
};

enum class FlashMode : uint8_t {
  Off,
  On,
  Auto,
  Motion
};

enum class LogLevel : uint8_t {
  Debug,
  Info,
  Warn,
  Error
};

enum class PhotoReason : uint8_t {
  ManualWeb,
  ManualTelegram,
  Motion
};

struct AppConfig {
  char hostname[32] = "esp32cam";
  char wifiSsid[33] = "";
  char wifiPass[65] = "";

  char telegramToken[112] = "";
  char telegramChatId[32] = "";

  char panelUser[20] = "admin";
  char panelPass[40] = "admin";

  ArmState startState = ArmState::Disarmed;
  FlashMode flashMode = FlashMode::Auto;

  uint8_t motionSensitivity = 5;
  uint16_t motionCooldownSec = 5;
  uint16_t minDetectIntervalMs = 900;
  uint8_t darkFlashThreshold = 130;
  uint8_t jpegQuality = 12;
  LogLevel logLevel = LogLevel::Info;

  bool savePhotosToSd = true;
  bool sendAlertsToTelegram = true;
  bool allowInsecureTls = APP_TELEGRAM_INSECURE_TLS_DEFAULT != 0;
};

struct AppState {
  ArmState armState = ArmState::Disarmed;
  bool wifiConnected = false;
  bool apMode = false;
  bool telegramReady = false;
  bool sdMounted = false;
  bool timeSynced = false;
  bool cameraReady = false;
  bool flashOn = false;

  uint32_t bootMillis = 0;
  uint32_t motionEvents = 0;
  uint32_t lastMotionMillis = 0;
  uint32_t lastMotionSampleMillis = 0;
  uint32_t lastPhotoMillis = 0;
  uint32_t lastTelegramMillis = 0;
  uint32_t lastWifiAttemptMillis = 0;

  uint16_t motionDiffSum = 0;
  uint8_t motionChangedBlocks = 0;
  uint8_t motionThreshold = 0;
  uint8_t motionMinBlocks = 0;
  uint8_t motionBrightness = 0;

  char lastPhotoPath[96] = "";
};

inline const char* armStateName(ArmState state) {
  switch (state) {
    case ArmState::Armed:
      return "armed";
    case ArmState::Privacy:
      return "privacy";
    case ArmState::Disarmed:
    default:
      return "disarmed";
  }
}

inline const char* flashModeName(FlashMode mode) {
  switch (mode) {
    case FlashMode::On:
      return "on";
    case FlashMode::Motion:
      return "motion";
    case FlashMode::Auto:
      return "auto";
    case FlashMode::Off:
    default:
      return "off";
  }
}
