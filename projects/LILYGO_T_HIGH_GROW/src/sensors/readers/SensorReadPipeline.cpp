#include "SensorReadPipeline.h"

#include "../../config/AppConfig.h"
#include "../../config/SensorConfig.h"
#include "../../system/Logging.h"

#include "../hw/SensorHardware.h"
#include "Bh1750Reader.h"
#include "Dht11Reader.h"
#include "SoilAdcReader.h"
#include "SaltAdcReader.h"
#include "BatteryAdcReader.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace SENSORS {

void SensorReadPipeline::readAll(SensorSnapshot& outSnap, PhaseStatus& outStatus) {
    outStatus.start_ms = millis();
    outStatus.ok = false;
    outStatus.error_code = "";
    outStatus.error_detail = "";

    SensorSnapshot snap;
    auto& cal = SensorConfig::get();

    bool gatedPower = SENSOR::POWER_ON_DEMAND;
    if (gatedPower) {
        SensorHardware::powerOnSensors();
        // Allow sensors (especially DHT) to stabilize after power-on
        vTaskDelay(pdMS_TO_TICKS(SENSOR::POWER_ON_STABILIZE_MS));
    }

    // Light
    {
        float lux = NAN;
        if (!Bh1750Reader::readLux(SensorHardware::lightMeter(), cal.luxOffset, lux)) {
            if (outStatus.error_code.isEmpty()) {
                outStatus.error_code = "BH1750_FAIL";
            }
        }
        snap.lux = lux;
    }
    vTaskDelay(pdMS_TO_TICKS(SENSOR::INTER_SENSOR_DELAY_MS));

    // Temperature
    {
        float temp = NAN;
        if (!Dht11Reader::readTemperature(SensorHardware::dht(), cal.tempOffset, temp)) {
            if (outStatus.error_code.isEmpty()) {
                outStatus.error_code = "DHT_TEMP_FAIL";
            }
        }
        snap.temp = temp;
    }
    vTaskDelay(pdMS_TO_TICKS(SENSOR::INTER_SENSOR_DELAY_MS));

    // Humidity
    {
        float humid = NAN;
        if (!Dht11Reader::readHumidity(SensorHardware::dht(), cal.humidOffset, humid)) {
            if (outStatus.error_code.isEmpty()) {
                outStatus.error_code = "DHT_HUMID_FAIL";
            }
        }
        snap.humid = humid;
    }
    vTaskDelay(pdMS_TO_TICKS(SENSOR::INTER_SENSOR_DELAY_MS));

    // Soil
    LOGI("Reading soil...");
    snap.soil = SoilAdcReader::readSoilPercent(HW::SOIL_PIN, cal.soilMin, cal.soilMax, cal.soilOffset);
    vTaskDelay(pdMS_TO_TICKS(SENSOR::INTER_SENSOR_DELAY_MS));

    // Salt
    LOGI("Reading salt (%d samples)...", (int)SENSOR::SALT_SAMPLES);
    snap.salt = SaltAdcReader::readSaltTrimmedMean(HW::SALT_PIN);
    LOGI("Salt OK (ADC=%d)", (int)snap.salt);
    vTaskDelay(pdMS_TO_TICKS(SENSOR::INTER_SENSOR_DELAY_MS));

    // Battery
    LOGI("Reading battery...");
    BatteryReading bat = BatteryAdcReader::read(HW::BAT_ADC, cal.batAdcMin, cal.batAdcMax);
    snap.batVolt = bat.volt;
    snap.batPerc = bat.perc;

    snap.timestamp_ms = millis();

    outStatus.duration_ms = millis() - outStatus.start_ms;
    outStatus.ok = outStatus.error_code.isEmpty();

    if (gatedPower) {
        SensorHardware::powerOffSensors();
    }

    outSnap = snap;
}

}  // namespace SENSORS
