#include "CameraService.h"

#include "BoardPins.h"
#include "Logger.h"

bool CameraService::begin(CameraProfile profile, uint8_t jpegQuality) {
  pinMode(BoardPins::FLASH_LED, OUTPUT);
  setFlash(false);
  return init(profile, jpegQuality);
}

bool CameraService::useProfile(CameraProfile profile, uint8_t jpegQuality) {
  if (initialized_ && profile_ == profile && jpegQuality_ == jpegQuality) {
    return true;
  }
  deinit();
  delay(60);
  return init(profile, jpegQuality);
}

bool CameraService::init(CameraProfile profile, uint8_t jpegQuality) {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = BoardPins::Y2;
  config.pin_d1 = BoardPins::Y3;
  config.pin_d2 = BoardPins::Y4;
  config.pin_d3 = BoardPins::Y5;
  config.pin_d4 = BoardPins::Y6;
  config.pin_d5 = BoardPins::Y7;
  config.pin_d6 = BoardPins::Y8;
  config.pin_d7 = BoardPins::Y9;
  config.pin_xclk = BoardPins::XCLK;
  config.pin_pclk = BoardPins::PCLK;
  config.pin_vsync = BoardPins::VSYNC;
  config.pin_href = BoardPins::HREF;
  config.pin_sccb_sda = BoardPins::SIOD;
  config.pin_sccb_scl = BoardPins::SIOC;
  config.pin_pwdn = BoardPins::PWDN;
  config.pin_reset = BoardPins::RESET;
  config.xclk_freq_hz = 20000000;
  config.fb_count = 1;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  if (profile == CameraProfile::Motion) {
    config.pixel_format = PIXFORMAT_GRAYSCALE;
    config.frame_size = FRAMESIZE_96X96;
    config.jpeg_quality = 16;
    config.fb_location = CAMERA_FB_IN_DRAM;
  } else {
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = psramFound() ? FRAMESIZE_SVGA : FRAMESIZE_QVGA;
    config.jpeg_quality = jpegQuality;
    config.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = ESP_FAIL;
  for (uint8_t attempt = 0; attempt < 3; ++attempt) {
    if (attempt > 0) {
      powerCycleSensor();
    }
    err = esp_camera_init(&config);
    if (err == ESP_OK) {
      break;
    }
    esp_camera_deinit();
    delay(120);
  }
  if (err != ESP_OK) {
    LOGE("CAM", "esp_camera_init failed");
    initialized_ = false;
    return false;
  }

  sensor_t* sensor = esp_camera_sensor_get();
  if (sensor != nullptr && profile == CameraProfile::Photo) {
    sensor->set_vflip(sensor, 0);
    sensor->set_hmirror(sensor, 0);
    sensor->set_brightness(sensor, 0);
    sensor->set_saturation(sensor, 0);
  }

  initialized_ = true;
  profile_ = profile;
  jpegQuality_ = jpegQuality;
  LOGI("CAM", profile == CameraProfile::Motion ? "motion profile ready" : "photo profile ready");
  return true;
}

void CameraService::deinit() {
  if (!initialized_) {
    return;
  }
  esp_camera_deinit();
  initialized_ = false;
}

void CameraService::powerCycleSensor() {
  if (BoardPins::PWDN < 0) {
    delay(120);
    return;
  }
  pinMode(BoardPins::PWDN, OUTPUT);
  digitalWrite(BoardPins::PWDN, HIGH);
  delay(30);
  digitalWrite(BoardPins::PWDN, LOW);
  delay(120);
}

bool CameraService::capture(camera_fb_t*& frame) {
  frame = nullptr;
  if (!initialized_) {
    return false;
  }
  frame = esp_camera_fb_get();
  if (frame == nullptr) {
    LOGW("CAM", "capture returned null framebuffer");
    return false;
  }
  return true;
}

void CameraService::release(camera_fb_t* frame) {
  if (frame != nullptr) {
    esp_camera_fb_return(frame);
  }
}

void CameraService::setFlash(bool on) {
  digitalWrite(BoardPins::FLASH_LED, on ? HIGH : LOW);
}

bool CameraService::ready() const {
  return initialized_;
}

CameraProfile CameraService::profile() const {
  return profile_;
}
