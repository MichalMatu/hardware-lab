#pragma once

#include <Arduino.h>

namespace SENSORS {

class SaltAdcReader {
public:
    static uint16_t readSaltTrimmedMean(uint8_t pin);
};

}  // namespace SENSORS
