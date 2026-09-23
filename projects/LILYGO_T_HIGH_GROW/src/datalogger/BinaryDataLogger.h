#pragma once

#include <Arduino.h>
#include <LittleFS.h>

namespace DATALOG {

/**
 * Binary data logger - writes sensor snapshots to daily .bin files
 * Uses BinaryLoggerHelpers for file operations
 */
class BinaryDataLogger {
public:
    static void begin();
    static void logSensorData(float temp, float humid, float lux, uint8_t soil, uint16_t salt, float batVolt, uint8_t batPerc);
    static void checkRotate();
};

}  // namespace DATALOG
