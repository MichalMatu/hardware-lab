#include "AppController.h"

#include <WiFi.h>
#include <esp_heap_caps.h>
#include <string.h>

#include "Logger.h"
#include "SafeString.h"

namespace {
constexpr uint32_t AmbientSampleMaxAgeMs = 15000;
constexpr uint16_t FlashSettleMs = 650;
}  // namespace

void AppController::begin() {
  state_.bootMillis = millis();
  Log.begin();
  LOGI("APP", "boot");
  LOGI("APP", APP_VERSION);

  cameraMutex_ = xSemaphoreCreateMutex();
  if (cameraMutex_ == nullptr) {
    LOGE("APP", "camera mutex failed");
  }
  telegramMutex_ = xSemaphoreCreateMutex();
  if (telegramMutex_ == nullptr) {
    LOGE("APP", "telegram mutex failed");
  }

  if (!configManager_.begin()) {
    LOGE("CFG", "Preferences begin failed");
  }
  configManager_.load(config_);
  Log.setLevel(config_.logLevel);
  state_.armState = config_.startState;

  state_.cameraReady = camera_.begin(CameraProfile::Photo, config_.jpegQuality);
  camera_.deinit();
  restoreFlashOutput();
  state_.sdMounted = storage_.begin(config_.savePhotosToSd);
  network_.begin(config_, state_);

  if (telegramConfigured()) {
    telegram_.begin(config_.telegramToken, config_.telegramChatId, config_.allowInsecureTls);
    state_.telegramReady = true;
  }

  web_.begin(config_, state_, configManager_, &AppController::captureThunk, &AppController::rebootThunk, &AppController::armStateThunk, this);

  xTaskCreatePinnedToCore(&AppController::webTaskThunk, "web", 4096, this, 1, &webTaskHandle_, 1);
  xTaskCreatePinnedToCore(&AppController::telegramTaskThunk, "telegram", 12288, this, 1, &telegramTaskHandle_, 1);
  xTaskCreatePinnedToCore(&AppController::motionTaskThunk, "motion", 6144, this, 1, &motionTaskHandle_, 1);

  Log.heap("APP");
}

void AppController::loop() {
  network_.ensureConnected(config_, state_);
  if (state_.wifiConnected && !state_.timeSynced) {
    network_.syncTime(state_);
  }
  delay(1000);
}

bool AppController::capturePhoto(PhotoReason reason, bool sendTelegram) {
  if (state_.armState == ArmState::Privacy && reason != PhotoReason::ManualWeb) {
    LOGW("APP", "privacy mode blocks capture");
    return false;
  }

  if (cameraMutex_ == nullptr || xSemaphoreTake(cameraMutex_, pdMS_TO_TICKS(30000)) != pdTRUE) {
    LOGW("APP", "camera busy");
    return false;
  }

  bool ok = false;
  camera_fb_t* frame = nullptr;
  uint8_t* telegramCopy = nullptr;
  size_t telegramCopyLen = 0;
  const bool flash = shouldUseFlashLocked(reason);
  bool flashStartedForCapture = false;

  do {
    if (!camera_.useProfile(CameraProfile::Photo, config_.jpegQuality)) {
      state_.cameraReady = false;
      break;
    }
    state_.cameraReady = true;
    if (flash && !state_.flashOn) {
      camera_.setFlash(true);
      state_.flashOn = true;
      flashStartedForCapture = true;
      delay(FlashSettleMs);
    }

    if (flashStartedForCapture) {
      camera_fb_t* staleFrame = nullptr;
      if (camera_.capture(staleFrame)) {
        camera_.release(staleFrame);
      }
      delay(60);
    }

    if (!camera_.capture(frame)) {
      break;
    }

    const char* category = reason == PhotoReason::Motion ? "motion" : "manual";
    if (config_.savePhotosToSd && state_.sdMounted) {
      storage_.savePhoto(category, frame->buf, frame->len, state_.lastPhotoPath, sizeof(state_.lastPhotoPath));
    }

    if (sendTelegram && state_.wifiConnected && telegramConfigured()) {
      char caption[96];
      snprintf(caption, sizeof(caption), "ESP32-CAM %s heap=%u", reasonName(reason), ESP.getFreeHeap());
      telegramCopy = static_cast<uint8_t*>(heap_caps_malloc(frame->len, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
      if (telegramCopy == nullptr) {
        LOGW("PHOTO", "PSRAM copy allocation failed");
      } else {
        memcpy(telegramCopy, frame->buf, frame->len);
        telegramCopyLen = frame->len;
        camera_.release(frame);
        frame = nullptr;
        camera_.deinit();
        state_.cameraReady = false;
      }

      const uint8_t* telegramData = telegramCopy != nullptr ? telegramCopy : frame->buf;
      const size_t telegramLen = telegramCopy != nullptr ? telegramCopyLen : frame->len;
      if (state_.flashOn && config_.flashMode != FlashMode::On) {
        camera_.setFlash(false);
        state_.flashOn = false;
      }
      if (sendTelegramPhoto(telegramData, telegramLen, caption)) {
        state_.lastTelegramMillis = millis();
        LOGI("TG", "photo sent");
      } else {
        LOGW("TG", "sendPhoto failed");
      }
    }

    state_.lastPhotoMillis = millis();
    ok = true;
  } while (false);

  if (frame != nullptr) {
    camera_.release(frame);
  }
  if (telegramCopy != nullptr) {
    heap_caps_free(telegramCopy);
  }
  camera_.deinit();
  restoreFlashOutput();
  xSemaphoreGive(cameraMutex_);
  Log.heap("PHOTO");
  return ok;
}

void AppController::reboot() {
  ESP.restart();
}

bool AppController::captureThunk(void* context, PhotoReason reason, bool sendTelegram) {
  return static_cast<AppController*>(context)->capturePhoto(reason, sendTelegram);
}

void AppController::rebootThunk(void* context) {
  static_cast<AppController*>(context)->reboot();
}

void AppController::armStateThunk(void* context, ArmState state) {
  static_cast<AppController*>(context)->setArmState(state);
}

void AppController::webTaskThunk(void* context) {
  static_cast<AppController*>(context)->webTask();
}

void AppController::telegramTaskThunk(void* context) {
  static_cast<AppController*>(context)->telegramTask();
}

void AppController::motionTaskThunk(void* context) {
  static_cast<AppController*>(context)->motionTask();
}

void AppController::webTask() {
  for (;;) {
    web_.handle();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void AppController::telegramTask() {
  TelegramCommand commands[4];
  for (;;) {
    if (network_.ensureConnected(config_, state_) && telegramConfigured()) {
      size_t count = 0;
      if (pollTelegram(commands, 4, count)) {
        for (size_t i = 0; i < count; ++i) {
          handleTelegramCommand(commands[i]);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void AppController::motionTask() {
  for (;;) {
    if (state_.armState == ArmState::Armed) {
      const uint32_t now = millis();
      const uint32_t cooldownMs = static_cast<uint32_t>(config_.motionCooldownSec) * 1000UL;
      if (state_.lastMotionMillis == 0 || now - state_.lastMotionMillis > cooldownMs) {
        bool detected = false;
        if (sampleMotion(detected) && detected) {
          state_.lastMotionMillis = millis();
          ++state_.motionEvents;
          Log.logf(LogLevel::Info, "MOTION", "detected blocks=%u/%u score=%u threshold=%u",
                   state_.motionChangedBlocks, state_.motionMinBlocks,
                   state_.motionDiffSum, state_.motionThreshold);
          if (config_.sendAlertsToTelegram) {
            capturePhoto(PhotoReason::Motion, true);
          } else {
            capturePhoto(PhotoReason::Motion, false);
          }
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(config_.minDetectIntervalMs));
  }
}

void AppController::setArmState(ArmState state) {
  state_.armState = state;
  if (state == ArmState::Armed) {
    state_.lastMotionMillis = 0;
    state_.motionEvents = 0;
    state_.lastMotionSampleMillis = 0;
    state_.motionChangedBlocks = 0;
    state_.motionDiffSum = 0;
    state_.motionThreshold = 0;
    state_.motionMinBlocks = 0;
    state_.motionBrightness = 0;
    motion_.reset();
    restoreFlashOutput();
    LOGI("MOTION", "armed");
    return;
  }

  restoreFlashOutput();
  LOGI("MOTION", state == ArmState::Privacy ? "privacy" : "disarmed");
}

void AppController::handleTelegramCommand(const TelegramCommand& command) {
  if (!telegram_.isAllowedChat(command.chatId)) {
    return;
  }

  if (strcmp(command.text, "/start") == 0 || strcmp(command.text, "/arm") == 0) {
    setArmState(ArmState::Armed);
    sendTelegramMessage("Armed");
  } else if (strcmp(command.text, "/stop") == 0 || strcmp(command.text, "/disarm") == 0) {
    setArmState(ArmState::Disarmed);
    sendTelegramMessage("Disarmed");
  } else if (strcmp(command.text, "/privacy") == 0) {
    setArmState(ArmState::Privacy);
    sendTelegramMessage("Privacy mode");
  } else if (strcmp(command.text, "/photo") == 0) {
    sendTelegramMessage("Capturing photo");
    capturePhoto(PhotoReason::ManualTelegram, true);
  } else if (strcmp(command.text, "/status") == 0) {
    sendStatus();
  } else if (strcmp(command.text, "/flash_on") == 0) {
    config_.flashMode = FlashMode::On;
    configManager_.save(config_);
    restoreFlashOutput();
    sendTelegramMessage("Flash on");
  } else if (strcmp(command.text, "/flash_off") == 0) {
    config_.flashMode = FlashMode::Off;
    configManager_.save(config_);
    restoreFlashOutput();
    sendTelegramMessage("Flash off");
  } else if (strcmp(command.text, "/flash_motion") == 0) {
    config_.flashMode = FlashMode::Motion;
    configManager_.save(config_);
    restoreFlashOutput();
    sendTelegramMessage("Flash motion");
  } else if (strcmp(command.text, "/flash_auto") == 0) {
    config_.flashMode = FlashMode::Auto;
    configManager_.save(config_);
    restoreFlashOutput();
    sendTelegramMessage("Flash auto");
  } else if (strcmp(command.text, "/reboot") == 0) {
    sendTelegramMessage("Rebooting");
    delay(300);
    reboot();
  } else {
    sendTelegramMessage("/photo /status /arm /disarm /privacy /flash_on /flash_off /flash_motion /flash_auto /reboot");
  }
}

void AppController::sendStatus() {
  char status[768];
  snprintf(status, sizeof(status),
           "state=%s\nwifi=%s\nip=%s\nsd=%s\ncamera=%s\nflash=%s led=%s brightness=%u/%u\nheap=%u\nmin_heap=%u\npsram=%u\nmotion_events=%lu\nmotion=%u/%u score=%u threshold=%u\nlast_motion=%lu\nlast_photo=%lu\nlast_path=%s",
           armStateName(state_.armState),
           state_.wifiConnected ? "connected" : "offline",
           state_.wifiConnected ? WiFi.localIP().toString().c_str() : "-",
           state_.sdMounted ? "mounted" : "off",
           state_.cameraReady ? "ready" : "error",
           flashModeName(config_.flashMode),
           state_.flashOn ? "on" : "off",
           state_.motionBrightness,
           config_.darkFlashThreshold,
           ESP.getFreeHeap(),
           ESP.getMinFreeHeap(),
           ESP.getFreePsram(),
           static_cast<unsigned long>(state_.motionEvents),
           state_.motionChangedBlocks,
           state_.motionMinBlocks,
           state_.motionDiffSum,
           state_.motionThreshold,
           state_.lastMotionMillis,
           state_.lastPhotoMillis,
           hasText(state_.lastPhotoPath) ? state_.lastPhotoPath : "-");
  sendTelegramMessage(status);
}

bool AppController::pollTelegram(TelegramCommand* commands, size_t maxCommands, size_t& count) {
  count = 0;
  if (telegramMutex_ == nullptr || xSemaphoreTake(telegramMutex_, pdMS_TO_TICKS(30000)) != pdTRUE) {
    LOGW("TG", "mutex timeout poll");
    return false;
  }
  if (!telegram_.ready()) {
    telegram_.begin(config_.telegramToken, config_.telegramChatId, config_.allowInsecureTls);
  }
  const bool ok = telegram_.poll(commands, maxCommands, count);
  xSemaphoreGive(telegramMutex_);
  return ok;
}

bool AppController::sendTelegramMessage(const char* text) {
  if (telegramMutex_ == nullptr || xSemaphoreTake(telegramMutex_, pdMS_TO_TICKS(30000)) != pdTRUE) {
    LOGW("TG", "mutex timeout message");
    return false;
  }
  const bool ok = telegram_.sendMessage(text);
  xSemaphoreGive(telegramMutex_);
  return ok;
}

bool AppController::sendTelegramPhoto(const uint8_t* jpeg, size_t length, const char* caption) {
  if (telegramMutex_ == nullptr || xSemaphoreTake(telegramMutex_, pdMS_TO_TICKS(45000)) != pdTRUE) {
    LOGW("TG", "mutex timeout photo");
    return false;
  }
  const bool ok = telegram_.sendPhoto(jpeg, length, caption);
  xSemaphoreGive(telegramMutex_);
  return ok;
}

bool AppController::sampleMotion(bool& motionDetected) {
  motionDetected = false;
  if (cameraMutex_ == nullptr || xSemaphoreTake(cameraMutex_, pdMS_TO_TICKS(5000)) != pdTRUE) {
    return false;
  }

  camera_fb_t* frame = nullptr;
  bool ok = false;
  do {
    if (!camera_.useProfile(CameraProfile::Motion, config_.jpegQuality)) {
      state_.cameraReady = false;
      break;
    }
    state_.cameraReady = true;
    if (!camera_.capture(frame)) {
      break;
    }
    motionDetected = motion_.analyzeGrayscale(frame->buf, frame->width, frame->height, config_.motionSensitivity);
    state_.lastMotionSampleMillis = millis();
    state_.motionChangedBlocks = motion_.changedBlocks();
    state_.motionDiffSum = motion_.diffSum();
    state_.motionThreshold = motion_.blockThreshold();
    state_.motionMinBlocks = motion_.minBlocks();
    state_.motionBrightness = motion_.brightness();
    ok = true;
  } while (false);

  if (frame != nullptr) {
    camera_.release(frame);
  }
  camera_.deinit();
  restoreFlashOutput();
  xSemaphoreGive(cameraMutex_);
  return ok;
}

void AppController::restoreFlashOutput() {
  const bool torch = config_.flashMode == FlashMode::On;
  camera_.setFlash(torch);
  state_.flashOn = torch;
}

bool AppController::shouldUseFlashLocked(PhotoReason reason) {
  switch (config_.flashMode) {
    case FlashMode::On:
      return true;
    case FlashMode::Motion:
      return reason == PhotoReason::Motion;
    case FlashMode::Auto: {
      uint8_t brightness = state_.motionBrightness;
      const uint32_t age = millis() - state_.lastMotionSampleMillis;
      const bool fresh = state_.lastMotionSampleMillis != 0 && age <= AmbientSampleMaxAgeMs;
      if (!fresh || reason != PhotoReason::Motion) {
        if (!measureBrightnessLocked(brightness)) {
          return false;
        }
      }
      const bool dark = brightness <= config_.darkFlashThreshold;
      Log.logf(LogLevel::Info, "FLASH", "auto brightness=%u threshold=%u %s",
               brightness, config_.darkFlashThreshold, dark ? "on" : "off");
      return dark;
    }
    case FlashMode::Off:
    default:
      return false;
  }
}

bool AppController::measureBrightnessLocked(uint8_t& brightness) {
  camera_fb_t* frame = nullptr;
  bool ok = false;
  do {
    if (!camera_.useProfile(CameraProfile::Motion, config_.jpegQuality)) {
      state_.cameraReady = false;
      break;
    }
    state_.cameraReady = true;
    if (!camera_.capture(frame)) {
      break;
    }

    uint32_t sum = 0;
    uint16_t count = 0;
    for (size_t i = 0; i < frame->len; i += 4) {
      sum += frame->buf[i];
      ++count;
    }
    brightness = count > 0 ? static_cast<uint8_t>(sum / count) : 0;
    state_.motionBrightness = brightness;
    state_.lastMotionSampleMillis = millis();
    ok = true;
  } while (false);

  if (frame != nullptr) {
    camera_.release(frame);
  }
  camera_.deinit();
  return ok;
}

bool AppController::telegramConfigured() const {
  return hasText(config_.telegramToken) && hasText(config_.telegramChatId);
}

const char* AppController::reasonName(PhotoReason reason) const {
  switch (reason) {
    case PhotoReason::Motion:
      return "motion";
    case PhotoReason::ManualTelegram:
      return "telegram";
    case PhotoReason::ManualWeb:
    default:
      return "web";
  }
}
