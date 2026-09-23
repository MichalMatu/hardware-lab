#pragma once

#include <Arduino.h>

// Sensor snapshot shared between task and API
struct SensorSnapshot {
    float lux = 0;
    float temp = 0;
    float humid = 0;
    uint8_t soil = 0;
    uint16_t salt = 0;
    uint8_t batPerc = 0;
    float batVolt = 0;
    uint32_t timestamp_ms = 0;
    uint32_t seq = 0;
};

// Phase status for detailed logging
struct PhaseStatus {
    bool ok = false;
    uint32_t start_ms = 0;
    uint32_t duration_ms = 0;
    String error_code = "";
    String error_detail = "";
};

struct ErrorInfo {
    String code = "";
    uint32_t timestamp_ms = 0;
};

// Commands for task queue
enum SensorTaskCommand : uint8_t {
    CMD_NONE = 0,
    CMD_FORCE_READ = 1,
    CMD_FORCE_LOG = 2,
    CMD_FORCE_READ_AND_LOG = 3
};
