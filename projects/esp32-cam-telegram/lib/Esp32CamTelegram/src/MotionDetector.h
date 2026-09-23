#pragma once

#include <Arduino.h>

class MotionDetector {
 public:
  bool analyzeGrayscale(const uint8_t* pixels, uint16_t width, uint16_t height, uint8_t sensitivity);
  void reset();
  uint8_t changedBlocks() const;
  uint16_t diffSum() const;
  uint8_t blockThreshold() const;
  uint8_t minBlocks() const;
  uint8_t brightness() const;

 private:
  static constexpr uint8_t GridX = 12;
  static constexpr uint8_t GridY = 9;
  static constexpr uint8_t CellCount = GridX * GridY;

  uint8_t baseline_[CellCount] = {};
  uint8_t warmupFrames_ = 0;
  uint8_t changedBlocks_ = 0;
  uint16_t diffSum_ = 0;
  uint8_t blockThreshold_ = 0;
  uint8_t minBlocks_ = 0;
  uint8_t brightness_ = 0;
  bool initialized_ = false;
};
