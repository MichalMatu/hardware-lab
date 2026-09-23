#pragma once

#include <Arduino.h>
#include <BH1750.h>

namespace SENSORS {

class Bh1750Reader {
public:
    static bool readLux(BH1750& meter, float luxOffset, float& outLux);
};

}  // namespace SENSORS
