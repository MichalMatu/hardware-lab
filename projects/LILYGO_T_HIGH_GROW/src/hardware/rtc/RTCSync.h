#ifndef HARDWARE_RTC_SYNC_H
#define HARDWARE_RTC_SYNC_H

#include "RTCCommon.h"

namespace RTCModule {
    namespace Sync {
        /**
         * @brief Synchronize RTC with ESP32 system time
         * 
         * Reads current system time (set by NTP) and updates RTC.
         * Only syncs if system time is valid (year in range 2025..2040).
         * 
         * @return true if synchronized successfully, false if system time invalid
         */
        bool syncWithSystemTime();
        
        /**
         * @brief Restore ESP32 system time from RTC (boot recovery)
         * 
         * Powers on RTC, reads DateTime, and sets ESP32 system time via settimeofday().
         * Use at boot to restore time when NTP/WiFi is unavailable.
         * Only sets system time if RTC time is valid (year in range 2025..2040).
         * 
         * @return true if system time was restored from RTC, false if RTC time invalid
         */
        bool syncSystemTimeFromRTC();

        /**
         * @brief Restore system time from RTC or set fallback if invalid
         *
         * Calls syncSystemTimeFromRTC(); if RTC time invalid, sets fallback 2025-01-01 UTC
         * to keep logs monotonic in battery_mode.
         *
         * @return true if restored from RTC, false if fallback was applied
         */
        bool restoreSystemTimeOrFallback();
        
        /**
         * @brief Get last successful sync timestamp
         * 
         * @return Unix timestamp of last RTC<->system sync, or 0 if never synced
         */
        time_t getLastSyncTime();
        
        /**
         * @brief Update last sync timestamp to current time
         * 
         * Called by NTP service after successful NTP sync + RTC update.
         */
        void updateLastSyncTime();
    }
}

#endif
