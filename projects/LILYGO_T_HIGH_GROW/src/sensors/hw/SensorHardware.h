#pragma once

#include <BH1750.h>
#include <DHT.h>

namespace SENSORS {

class SensorHardware {
public:
    static bool ensureInitialized();

    static void powerOnSensors();
    static void powerOffSensors();

    static BH1750& lightMeter();
    static DHT& dht();

private:
    static bool _initialized;
};

}  // namespace SENSORS
