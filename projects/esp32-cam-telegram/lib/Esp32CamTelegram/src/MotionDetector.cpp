#include "MotionDetector.h"

#include <stdlib.h>
#include <string.h>

bool MotionDetector::analyzeGrayscale(const uint8_t* pixels, uint16_t width, uint16_t height, uint8_t sensitivity) {
  if (pixels == nullptr || width < GridX || height < GridY) {
    return false;
  }

  if (sensitivity < 1) {
    sensitivity = 1;
  } else if (sensitivity > 10) {
    sensitivity = 10;
  }

  changedBlocks_ = 0;
  diffSum_ = 0;
  const int threshold = 25 - static_cast<int>(sensitivity) * 2;
  blockThreshold_ = threshold < 5 ? 5 : static_cast<uint8_t>(threshold);
  const int blocks = 12 - sensitivity;
  minBlocks_ = blocks < 2 ? 2 : static_cast<uint8_t>(blocks);

  uint8_t cells[CellCount] = {};
  const uint16_t blockW = width / GridX;
  const uint16_t blockH = height / GridY;

  uint16_t cellIndex = 0;
  uint32_t globalCurrent = 0;
  uint32_t globalBase = 0;
  for (uint8_t by = 0; by < GridY; ++by) {
    for (uint8_t bx = 0; bx < GridX; ++bx) {
      uint32_t sum = 0;
      uint16_t count = 0;
      const uint16_t x0 = bx * blockW;
      const uint16_t y0 = by * blockH;
      for (uint16_t y = y0; y < y0 + blockH; y += 2) {
        const uint32_t row = static_cast<uint32_t>(y) * width;
        for (uint16_t x = x0; x < x0 + blockW; x += 2) {
          sum += pixels[row + x];
          ++count;
        }
      }
      cells[cellIndex] = count > 0 ? static_cast<uint8_t>(sum / count) : 0;
      globalCurrent += cells[cellIndex];
      globalBase += baseline_[cellIndex];
      ++cellIndex;
    }
  }
  brightness_ = static_cast<uint8_t>(globalCurrent / CellCount);

  if (!initialized_) {
    memcpy(baseline_, cells, sizeof(baseline_));
    initialized_ = true;
    warmupFrames_ = 3;
    return false;
  }

  if (warmupFrames_ > 0) {
    memcpy(baseline_, cells, sizeof(baseline_));
    --warmupFrames_;
    return false;
  }

  const int16_t globalDelta = static_cast<int16_t>(globalCurrent / CellCount) -
                              static_cast<int16_t>(globalBase / CellCount);
  for (uint8_t i = 0; i < CellCount; ++i) {
    const int16_t normalizedCurrent = static_cast<int16_t>(cells[i]) - globalDelta;
    const int16_t diff = abs(normalizedCurrent - static_cast<int16_t>(baseline_[i]));
    if (diff > blockThreshold_) {
      ++changedBlocks_;
      diffSum_ += static_cast<uint16_t>(diff);
    }
  }

  const bool motion = changedBlocks_ >= minBlocks_ &&
                      diffSum_ >= static_cast<uint16_t>(minBlocks_ * blockThreshold_);

  const uint8_t alphaBase = motion ? 15 : 7;
  for (uint8_t i = 0; i < CellCount; ++i) {
    baseline_[i] = static_cast<uint8_t>((static_cast<uint16_t>(baseline_[i]) * alphaBase + cells[i]) / (alphaBase + 1));
  }

  return motion;
}

void MotionDetector::reset() {
  memset(baseline_, 0, sizeof(baseline_));
  warmupFrames_ = 0;
  changedBlocks_ = 0;
  diffSum_ = 0;
  blockThreshold_ = 0;
  minBlocks_ = 0;
  brightness_ = 0;
  initialized_ = false;
}

uint8_t MotionDetector::changedBlocks() const {
  return changedBlocks_;
}

uint16_t MotionDetector::diffSum() const {
  return diffSum_;
}

uint8_t MotionDetector::blockThreshold() const {
  return blockThreshold_;
}

uint8_t MotionDetector::minBlocks() const {
  return minBlocks_;
}

uint8_t MotionDetector::brightness() const {
  return brightness_;
}
