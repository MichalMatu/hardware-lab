#pragma once

#include <Arduino.h>

namespace SENSORS {

struct BatteryReading {
    uint8_t perc = 0;
    float volt = 0;
    uint16_t adc = 0;
};

class BatteryAdcReader {
public:
    static BatteryReading read(uint8_t pin, int batAdcMin, int batAdcMax);
};

}  // namespace SENSORS
