#pragma once

#include "../model/SensorTypes.h"

namespace SENSORS {

class SensorReadPipeline {
public:
    // Reads all sensors, applies calibration, sets seq/timestamps.
    // NOTE: Does not update global state; caller decides when to publish snapshot.
    static void readAll(SensorSnapshot& outSnap, PhaseStatus& outStatus);
};

}  // namespace SENSORS
