#pragma once

#include "../model/SensorTypes.h"

namespace SENSORS {

class SensorBinaryLogger {
public:
    static void writeSnapshot(const SensorSnapshot& snap, PhaseStatus& outStatus);
};

}  // namespace SENSORS
