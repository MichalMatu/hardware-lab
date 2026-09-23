#pragma once

#include <Arduino.h>

namespace SENSORS {

class SoilAdcReader {
public:
    static uint8_t readSoilPercent(uint8_t pin, int soilMin, int soilMax, float soilOffset);
};

}  // namespace SENSORS
