#include "ConfigManager.h"

#include "SafeString.h"

namespace {
uint8_t enumByte(LogLevel value) {
  return static_cast<uint8_t>(value);
}

uint8_t enumByte(ArmState value) {
  return static_cast<uint8_t>(value);
}

uint8_t enumByte(FlashMode value) {
  return static_cast<uint8_t>(value);
}
}  // namespace

bool ConfigManager::begin() {
  return prefs_.begin("camapp", false);
}

bool ConfigManager::load(AppConfig& config) {
  copyText(config.hostname, sizeof(config.hostname), prefs_.getString("host", config.hostname));
  copyText(config.wifiSsid, sizeof(config.wifiSsid), prefs_.getString("ssid", config.wifiSsid));
  copyText(config.wifiPass, sizeof(config.wifiPass), prefs_.getString("wpass", config.wifiPass));
  copyText(config.telegramToken, sizeof(config.telegramToken), prefs_.getString("tgtok", config.telegramToken));
  copyText(config.telegramChatId, sizeof(config.telegramChatId), prefs_.getString("tgchat", config.telegramChatId));
  copyText(config.panelUser, sizeof(config.panelUser), prefs_.getString("puser", config.panelUser));
  copyText(config.panelPass, sizeof(config.panelPass), prefs_.getString("ppass", config.panelPass));

  const uint8_t armValue = prefs_.getUChar("arm", enumByte(config.startState));
  config.startState = armValue <= enumByte(ArmState::Privacy) ? static_cast<ArmState>(armValue) : ArmState::Disarmed;
  const uint8_t flashValue = prefs_.getUChar("flash", enumByte(config.flashMode));
  config.flashMode = flashValue <= enumByte(FlashMode::Motion) ? static_cast<FlashMode>(flashValue) : FlashMode::Auto;
  config.motionSensitivity = prefs_.getUChar("msens", config.motionSensitivity);
  config.motionCooldownSec = prefs_.getUShort("mcool", config.motionCooldownSec);
  config.minDetectIntervalMs = prefs_.getUShort("mint", config.minDetectIntervalMs);
  config.darkFlashThreshold = prefs_.getUChar("darkth", config.darkFlashThreshold);
  config.jpegQuality = prefs_.getUChar("jpeg", config.jpegQuality);
  config.logLevel = static_cast<LogLevel>(prefs_.getUChar("log", enumByte(config.logLevel)));
  config.savePhotosToSd = prefs_.getBool("savesd", config.savePhotosToSd);
  config.sendAlertsToTelegram = prefs_.getBool("tgalerts", config.sendAlertsToTelegram);
  config.allowInsecureTls = prefs_.getBool("tlsinsec", config.allowInsecureTls);

  if (config.motionSensitivity < 1) {
    config.motionSensitivity = 1;
  }
  if (config.motionSensitivity > 10) {
    config.motionSensitivity = 10;
  }
  if (config.motionCooldownSec < 2) {
    config.motionCooldownSec = 2;
  }
  if (config.jpegQuality < 8) {
    config.jpegQuality = 8;
  }
  if (config.jpegQuality > 20) {
    config.jpegQuality = 20;
  }
  if (config.darkFlashThreshold < 5) {
    config.darkFlashThreshold = 5;
  }
  if (config.darkFlashThreshold > 220) {
    config.darkFlashThreshold = 220;
  }
  return true;
}

bool ConfigManager::save(const AppConfig& config) {
  prefs_.putString("host", config.hostname);
  prefs_.putString("ssid", config.wifiSsid);
  prefs_.putString("wpass", config.wifiPass);
  prefs_.putString("tgtok", config.telegramToken);
  prefs_.putString("tgchat", config.telegramChatId);
  prefs_.putString("puser", config.panelUser);
  prefs_.putString("ppass", config.panelPass);
  prefs_.putUChar("arm", enumByte(config.startState));
  prefs_.putUChar("flash", enumByte(config.flashMode));
  prefs_.putUChar("msens", config.motionSensitivity);
  prefs_.putUShort("mcool", config.motionCooldownSec);
  prefs_.putUShort("mint", config.minDetectIntervalMs);
  prefs_.putUChar("darkth", config.darkFlashThreshold);
  prefs_.putUChar("jpeg", config.jpegQuality);
  prefs_.putUChar("log", enumByte(config.logLevel));
  prefs_.putBool("savesd", config.savePhotosToSd);
  prefs_.putBool("tgalerts", config.sendAlertsToTelegram);
  prefs_.putBool("tlsinsec", config.allowInsecureTls);
  return true;
}

bool ConfigManager::clear() {
  return prefs_.clear();
}
