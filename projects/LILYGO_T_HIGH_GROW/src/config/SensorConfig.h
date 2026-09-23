#ifndef SensorConfig_h
#define SensorConfig_h

#include <Preferences.h>

struct SensorCalibration {
    float tempOffset = 0.0;
    float humidOffset = 0.0;
    float luxOffset = 0.0;
    float soilOffset = 0.0;
    int soilMin = 1391;
    int soilMax = 3300;
    // LiPo battery with 1:2 voltage divider (100kΩ + 100kΩ)
    // 3.0V (empty) → 1.5V @ ADC → ~1860 ADC
    // 4.35V (full, freshly charged) → 2.175V @ ADC → ~2700 ADC
    int batAdcMin = 1860;  // 3.0V - bezpieczne minimum dla LiPo
    int batAdcMax = 2700;  // 4.35V - świeżo naładowana LiPo (spada do 4.2V w użyciu)
};

class SensorConfig {
public:
    static void begin();
    static void load();
    static void save();
    static SensorCalibration& get() { return _config; }

private:
    static SensorCalibration _config;
    static Preferences _prefs;
};

#endif
