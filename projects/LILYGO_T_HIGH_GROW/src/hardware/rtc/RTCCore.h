#ifndef HARDWARE_RTC_CORE_H
#define HARDWARE_RTC_CORE_H

#include "RTCCommon.h"

namespace RTCModule {
    namespace Core {
        /**
         * @brief Initialize RTC module
         * 
         * Sets up GPIO for power control, initializes Wire1 bus,
         * and attempts to communicate with DS3231.
         * 
         * @return true if RTC initialized successfully, false otherwise
         */
        bool begin();
        
        /**
         * @brief Read current time from RTC and print to log
         * 
         * Powers on RTC, reads DateTime from DS3231,
         * prints formatted timestamp to log, then powers off.
         * 
         * Format: "[RTC] YYYY-MM-DD HH:MM:SS UTC"
         */
        void readAndPrint();
        
        /**
         * @brief Set RTC time from Unix timestamp (internal, assumes mutex held)
         * 
         * Powers on RTC, sets the DS3231 to the specified Unix time,
         * then powers off. Caller must hold rtcMutex.
         * 
         * @param unixTime Unix timestamp (seconds since 1970-01-01 00:00:00 UTC)
         * @return true if time was set successfully, false otherwise
         */
        bool setTimeInternal(time_t unixTime);
        
        /**
         * @brief Set RTC time from Unix timestamp (thread-safe)
         * 
         * Acquires mutex, powers on RTC, sets time, and powers off.
         * Used for NTP synchronization.
         * 
         * @param unixTime Unix timestamp (seconds since 1970-01-01 00:00:00 UTC)
         * @return true if time was set successfully, false otherwise
         */
        bool setTime(time_t unixTime);
    }
}

#endif
