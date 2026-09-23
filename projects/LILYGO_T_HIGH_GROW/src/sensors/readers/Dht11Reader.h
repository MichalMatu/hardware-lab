#pragma once

#include <Arduino.h>
#include <DHT.h>

namespace SENSORS {

class Dht11Reader {
public:
    static bool readTemperature(DHT& dht, float tempOffset, float& outTemp);
    static bool readHumidity(DHT& dht, float humidOffset, float& outHumid);
};

}  // namespace SENSORS
