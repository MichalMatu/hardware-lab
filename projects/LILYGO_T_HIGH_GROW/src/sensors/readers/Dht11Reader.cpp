#include "Dht11Reader.h"

#include "../../config/AppConfig.h"
#include "../../system/Logging.h"

#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace SENSORS {

bool Dht11Reader::readTemperature(DHT& dht, float tempOffset, float& outTemp) {
    for (int attempt = 1; attempt <= SENSOR::MAX_READ_ATTEMPTS; attempt++) {
        float reading = dht.readTemperature();
        if (!isnan(reading)) {
            LOGD("DHT11: temp attempt %d OK (%.1f C)", attempt, reading);
            outTemp = reading + tempOffset;
            return true;
        }
        LOGW("DHT11: temp attempt %d FAIL (NaN)", attempt);
        vTaskDelay(pdMS_TO_TICKS(SENSOR::READ_RETRY_DELAY_MS));
    }

    outTemp = NAN;
    return false;
}

bool Dht11Reader::readHumidity(DHT& dht, float humidOffset, float& outHumid) {
    for (int attempt = 1; attempt <= SENSOR::MAX_READ_ATTEMPTS; attempt++) {
        float reading = dht.readHumidity();
        if (!isnan(reading)) {
            LOGD("DHT11: humid attempt %d OK (%.1f %%)", attempt, reading);
            outHumid = reading + humidOffset;
            return true;
        }
        LOGW("DHT11: humid attempt %d FAIL (NaN)", attempt);
        vTaskDelay(pdMS_TO_TICKS(SENSOR::READ_RETRY_DELAY_MS));
    }

    outHumid = NAN;
    return false;
}

}  // namespace SENSORS
