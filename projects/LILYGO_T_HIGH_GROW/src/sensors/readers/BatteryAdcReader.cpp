#include "BatteryAdcReader.h"

#include "../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace SENSORS {

BatteryReading BatteryAdcReader::read(uint8_t pin, int batAdcMin, int batAdcMax) {
    BatteryReading r;

    // Average a few samples to reduce ADC noise.
    constexpr uint8_t kSamples = 8;
    uint32_t adcSum = 0;
    uint32_t mvSum = 0;
    for (uint8_t i = 0; i < kSamples; i++) {
        adcSum += (uint32_t)analogRead(pin);
        mvSum += (uint32_t)analogReadMilliVolts(pin);
    }
    r.adc = (uint16_t)((adcSum + (kSamples / 2)) / kSamples);

    // TTGO T-HIGrow has 1:2 voltage divider (100kΩ + 100kΩ).
    // analogReadMilliVolts() returns millivolts at the ADC pin (after divider),
    // so battery voltage is ~2x that.
    const uint32_t adcMv = (mvSum + (kSamples / 2)) / kSamples;
    r.volt = (adcMv * 2.0f) / 1000.0f;
    r.perc = (uint8_t)constrain(map(r.adc, batAdcMin, batAdcMax, 0, 100), 0, 100);

    LOGI("BAT: OK (%d %%, %.2fV, ADC=%d)", r.perc, r.volt, (int)r.adc);
    return r;
}

}  // namespace SENSORS
