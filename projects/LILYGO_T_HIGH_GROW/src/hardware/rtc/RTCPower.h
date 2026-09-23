#ifndef HARDWARE_RTC_POWER_H
#define HARDWARE_RTC_POWER_H

#include "RTCCommon.h"

namespace RTCModule {
    namespace Power {
        /**
         * @brief Power on RTC module and wait for stabilization
         * 
         * Sets RTC_POWER GPIO HIGH and waits for DS3231 to stabilize.
         * Must be called before any I2C communication with RTC.
         */
        void powerOn();
        
        /**
         * @brief Power off RTC module to save battery
         * 
         * Sets RTC_POWER GPIO LOW. Wire1 I2C bus remains active.
         * Call after completing read/write operations.
         */
        void powerOff();
    }
}

#endif
