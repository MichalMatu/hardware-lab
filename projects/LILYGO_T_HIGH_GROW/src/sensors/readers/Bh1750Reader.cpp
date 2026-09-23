#include "Bh1750Reader.h"

#include "../../config/AppConfig.h"
#include "../../system/Logging.h"

#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace SENSORS {

bool Bh1750Reader::readLux(BH1750& meter, float luxOffset, float& outLux) {
    for (int attempt = 1; attempt <= SENSOR::MAX_READ_ATTEMPTS; attempt++) {
        float reading = meter.readLightLevel();
        if (!isnan(reading)) {
            LOGD("BH1750: attempt %d OK (%.1f lux)", attempt, reading);
            outLux = reading + luxOffset;
            return true;
        }
        LOGW("BH1750: attempt %d FAIL (NaN)", attempt);
        vTaskDelay(pdMS_TO_TICKS(SENSOR::READ_RETRY_DELAY_MS));
    }

    outLux = NAN;
    return false;
}

}  // namespace SENSORS
