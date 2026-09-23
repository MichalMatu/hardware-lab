#include "SensorConfig.h"
#include "../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "Cfg"

SensorCalibration SensorConfig::_config;
Preferences SensorConfig::_prefs;

void SensorConfig::begin() {
    _prefs.begin("sensor_cal", false);
    load();
}

void SensorConfig::load() {
    _config.tempOffset = _prefs.isKey("temp_off") ? _prefs.getFloat("temp_off") : 0.0f;
    _config.humidOffset = _prefs.isKey("humid_off") ? _prefs.getFloat("humid_off") : 0.0f;
    _config.luxOffset = _prefs.isKey("lux_off") ? _prefs.getFloat("lux_off") : 0.0f;
    _config.soilOffset = _prefs.isKey("soil_off") ? _prefs.getFloat("soil_off") : 0.0f;
    _config.soilMin = _prefs.isKey("soil_min") ? _prefs.getInt("soil_min") : 1391;
    _config.soilMax = _prefs.isKey("soil_max") ? _prefs.getInt("soil_max") : 3300;
    _config.batAdcMin = _prefs.isKey("bat_min") ? _prefs.getInt("bat_min") : _config.batAdcMin;
    _config.batAdcMax = _prefs.isKey("bat_max") ? _prefs.getInt("bat_max") : _config.batAdcMax;
    
    LOGI("Calibration loaded");
}

void SensorConfig::save() {
    _prefs.putFloat("temp_off", _config.tempOffset);
    _prefs.putFloat("humid_off", _config.humidOffset);
    _prefs.putFloat("lux_off", _config.luxOffset);
    _prefs.putFloat("soil_off", _config.soilOffset);
    _prefs.putInt("soil_min", _config.soilMin);
    _prefs.putInt("soil_max", _config.soilMax);
    _prefs.putInt("bat_min", _config.batAdcMin);
    _prefs.putInt("bat_max", _config.batAdcMax);
    
    LOGI("Calibration saved");
}
