#pragma once

#include <Arduino.h>
#include <esp_camera.h>

enum class CameraProfile : uint8_t {
  Photo,
  Motion
};

class CameraService {
 public:
  bool begin(CameraProfile profile, uint8_t jpegQuality);
  bool useProfile(CameraProfile profile, uint8_t jpegQuality);
  bool capture(camera_fb_t*& frame);
  void release(camera_fb_t* frame);
  void deinit();
  void setFlash(bool on);
  bool ready() const;
  CameraProfile profile() const;

 private:
  bool init(CameraProfile profile, uint8_t jpegQuality);
  void powerCycleSensor();

  bool initialized_ = false;
  CameraProfile profile_ = CameraProfile::Photo;
  uint8_t jpegQuality_ = 12;
};
