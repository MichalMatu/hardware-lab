#include "SoilAdcReader.h"

#include "../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace SENSORS {

uint8_t SoilAdcReader::readSoilPercent(uint8_t pin, int soilMin, int soilMax, float soilOffset) {
    int soilRaw = analogRead(pin);
    uint8_t soil = constrain(map(soilRaw, soilMin, soilMax, 100, 0), 0, 100);

    int adjusted = (int)roundf((float)soil + soilOffset);
    soil = (uint8_t)constrain(adjusted, 0, 100);

    LOGI("SOIL: OK (%d %%, raw=%d)", soil, soilRaw);
    return soil;
}

}  // namespace SENSORS
