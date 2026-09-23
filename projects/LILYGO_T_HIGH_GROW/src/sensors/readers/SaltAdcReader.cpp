#include "SaltAdcReader.h"

#include "../../config/AppConfig.h"
#include "../../system/Logging.h"

#include <algorithm>
#include <array>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace SENSORS {

uint16_t SaltAdcReader::readSaltTrimmedMean(uint8_t pin) {
    const bool debugEnabled = LOG::Logging::isEnabled(ESP_LOG_DEBUG);
    LOGD("SALT: read pin=%d, samples=%d", pin, (int)SENSOR::SALT_SAMPLES);

    std::array<uint16_t, SENSOR::SALT_SAMPLES> samples{};

    for (size_t i = 0; i < samples.size(); i++) {
        samples[i] = analogRead(pin);
        if (debugEnabled && (i < 3 || i >= samples.size() - 3)) {
            LOGD("SALT: sample[%d]=%d", (int)i, (int)samples[i]);
        }
        vTaskDelay(pdMS_TO_TICKS(SENSOR::SALT_SAMPLE_DELAY_MS));
    }

    std::sort(samples.begin(), samples.end());

    if (debugEnabled) {
        LOGD("SALT: sorted min=%d, max=%d", (int)samples.front(), (int)samples.back());
    }

    uint32_t sum = 0;
    int count = 0;
    for (size_t i = 0; i < samples.size(); i++) {
        if (i == 0 || i == samples.size() - 1) {
            continue;  // Skip min/max (trimmed mean)
        }
        sum += samples[i];
        count++;
    }

    uint16_t result = (count > 0) ? (uint16_t)(sum / (uint32_t)count) : 0;
    LOGD("SALT: trimmed mean (n=%d): avg=%u", count, (unsigned)result);
    return result;
}

}  // namespace SENSORS
