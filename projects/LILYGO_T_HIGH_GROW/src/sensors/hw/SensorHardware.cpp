#include "SensorHardware.h"

#include "../../config/AppConfig.h"
#include "../../system/Logging.h"

#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace {
BH1750 g_lightMeter(SENSOR::BH1750_ADDR);
DHT g_dht(HW::DHT_PIN, DHT11);
}

namespace SENSORS {

bool SensorHardware::_initialized = false;

bool SensorHardware::ensureInitialized() {
    if (_initialized) {
        return true;
    }

    pinMode(HW::POWER_CTRL, OUTPUT);

    // Always power on for initialization to ensure I2C bus is stable
    // If sensors are off, I2C lines might be pulled low by unpowered chips, 
    // causing the ESP32 I2C peripheral to enter an error state.
    digitalWrite(HW::POWER_CTRL, HIGH);
    vTaskDelay(pdMS_TO_TICKS(SENSOR::POWER_STABILIZE_DELAY_MS));

    // ESP32 I2C peripheral reset: end() + begin() cycle to clear any stuck state.
    // Crucial for the new i2c-ng driver in Arduino-ESP32 v3.x.
    Wire.end();
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Initialize I2C with 50kHz for maximum reliability during discovery.
    // BH1750 and other sensors on this bus are sensitive to timing during power-up.
    if (!Wire.begin(HW::I2C_SDA, HW::I2C_SCL, 50000)) {
        LOGE("HW: I2C0 begin failed!");
    }
    
    // Increase timeout for the new driver to handle slow sensor wake-ups.
    Wire.setTimeOut(100); 
    vTaskDelay(pdMS_TO_TICKS(100));

    // Give BH1750 extra time to wake after power gate to avoid I2C timeouts
    vTaskDelay(pdMS_TO_TICKS(SENSOR::BH1750_INIT_DELAY_MS));

    bool bh1750Ok = g_lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
    
    // Retry once with a fresh start if first attempt fails
    if (!bh1750Ok) {
        LOGW("HW: BH1750 init failed, retrying...");
        vTaskDelay(pdMS_TO_TICKS(200));
        bh1750Ok = g_lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
    }
    
    if (bh1750Ok) {
        LOGI("HW: BH1750 initialized");
    } else {
        LOGW("HW: BH1750 init failed after retry");
    }

    g_dht.begin();
    LOGI("HW: DHT11 initialized");

    if (SENSOR::POWER_ON_DEMAND) {
        powerOffSensors();
    }

    // ADC setup: make behavior explicit (and help analogReadMilliVolts use the right attenuation).
    analogReadResolution(12);
    // Note: in this Arduino-ESP32 core the default attenuation is already 11dB;
    // configuring per-pin attenuation before the first analogRead() can emit noisy boot logs.
    LOGD("HW: ADC ready (12-bit, default attenuation)");

    _initialized = true;
    return true;
}

void SensorHardware::powerOnSensors() {
    digitalWrite(HW::POWER_CTRL, HIGH);
    vTaskDelay(pdMS_TO_TICKS(SENSOR::POWER_STABILIZE_DELAY_MS));
    // Extra delay for ESP32 I2C peripheral to stabilize after power on
    vTaskDelay(pdMS_TO_TICKS(100));
    g_lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
    // BH1750 needs time to stabilize after begin() before first read
    vTaskDelay(pdMS_TO_TICKS(SENSOR::BH1750_INIT_DELAY_MS));
    g_dht.begin();
}

void SensorHardware::powerOffSensors() {
    digitalWrite(HW::POWER_CTRL, LOW);
}

BH1750& SensorHardware::lightMeter() {
    return g_lightMeter;
}

DHT& SensorHardware::dht() {
    return g_dht;
}

}  // namespace SENSORS
