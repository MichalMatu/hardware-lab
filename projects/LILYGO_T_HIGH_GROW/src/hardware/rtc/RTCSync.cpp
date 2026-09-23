#include "RTCSync.h"
#include "RTCCore.h"
#include "RTCPower.h"
#include "../../system/Logging.h"
#include "../../config/AppConfig.h"

#undef LOG_TAG
#define LOG_TAG "RTC:Sync"

using namespace RTC;
using namespace RTCModule::Internal;

namespace RTCModule {
    namespace Sync {
        bool syncWithSystemTime() {
            if (!initialized) {
                LOGE("Module not initialized");
                return false;
            }

            if (!lockRtc()) {
                LOGE("Mutex timeout in syncWithSystemTime");
                return false;
            }
            
            // Get current system time (set by NTP) - Unix timestamp is always UTC
            time_t now = time(nullptr);
            struct tm timeinfo_utc;
            gmtime_r(&now, &timeinfo_utc);
            int year = timeinfo_utc.tm_year + 1900;
            
            // Check if system time is valid (range 2025-2040)
            if (!isYearValid(year)) {
                unlockRtc();
                LOGW("System time invalid (year=%d), skipping sync", year);
                return false;
            }
            
            // Power on RTC for reading current time
            Power::powerOn();
            
            // Read current RTC time for debug (UTC)
            DateTime rtcBefore = rtc.now();
            
            // Power off before write operation
            Power::powerOff();

            LOGI("RTC sync: before=%04d-%02d-%02dT%02d:%02d:%02dZ system=%04d-%02d-%02dT%02d:%02d:%02dZ",
                 rtcBefore.year(), rtcBefore.month(), rtcBefore.day(),
                 rtcBefore.hour(), rtcBefore.minute(), rtcBefore.second(),
                 timeinfo_utc.tm_year + 1900, timeinfo_utc.tm_mon + 1, timeinfo_utc.tm_mday,
                 timeinfo_utc.tm_hour, timeinfo_utc.tm_min, timeinfo_utc.tm_sec);
            
            bool ok = Core::setTimeInternal(now);
            if (ok) {
                lastSyncTime = now;
                // Power on RTC for verification read
                Power::powerOn();
                
                DateTime rtcAfter = rtc.now();
                LOGI("RTC sync done: after=%04d-%02d-%02dT%02d:%02d:%02dZ",
                     rtcAfter.year(), rtcAfter.month(), rtcAfter.day(),
                     rtcAfter.hour(), rtcAfter.minute(), rtcAfter.second());
                
                // Power off after verification
                Power::powerOff();
            }
            unlockRtc();
            return ok;
        }
        
        bool syncSystemTimeFromRTC() {
            if (!initialized) {
                LOGE("Module not initialized");
                return false;
            }

            if (!lockRtc()) {
                LOGE("Mutex timeout in syncSystemTimeFromRTC");
                return false;
            }
            
            // Power on RTC
            Power::powerOn();
            
            // ESP32 I2C peripheral reset: clear any stuck state from Wire1.
            // Crucial for the new i2c-ng driver in Arduino-ESP32 v3.x.
            Wire1.end();
            vTaskDelay(pdMS_TO_TICKS(100));
            
            // Initialize I2C1 with 50kHz for maximum reliability.
            // DS3231 is robust but the ESP32 peripheral needs a clean start.
            if (!Wire1.begin(HW::RTC_SDA, HW::RTC_SCL, 50000)) {
                LOGE("HW: I2C1 begin failed!");
            }
            
            // Increase timeout for the new driver to handle slow wake-ups.
            Wire1.setTimeOut(100);
            vTaskDelay(pdMS_TO_TICKS(100));
            
            // Read current time from RTC
            DateTime rtcTime = rtc.now();

            // Quick retry on invalid read (hardware timeout/garbage after wake)
            int year = rtcTime.year();
            if (!isYearValid(year)) {
                LOGW("RTC time invalid on first read (year=%d), retrying", year);
                Power::powerOff();
                // Longer off-time to fully discharge any stray capacitance
                vTaskDelay(pdMS_TO_TICKS(RTC::POWER_STABILIZE_DELAY_MS + 50));
                Power::powerOn();
                // Extra settling time before retry read
                vTaskDelay(pdMS_TO_TICKS(50));
                rtcTime = rtc.now();
                year = rtcTime.year();
            }
            
            // Power off immediately after read
            Power::powerOff();
            
            // Validate RTC time (range 2025-2040)
            if (!isYearValid(year)) {
                unlockRtc();
                LOGW("RTC time invalid (year=%d), cannot restore system time", year);
                return false;
            }
            
            // Debug old system time (UTC)
            time_t sysBefore = time(nullptr);
            struct tm sysBeforeUtc;
            gmtime_r(&sysBefore, &sysBeforeUtc);

            // Set ESP32 system time from RTC (RTC stores UTC, system time will handle timezone)
            time_t rtcUnixTime = rtcTime.unixtime();
            struct timeval now_tv = {.tv_sec = rtcUnixTime, .tv_usec = 0};
            settimeofday(&now_tv, nullptr);

            LOGI("System restore from RTC: old=%04d-%02d-%02dT%02d:%02d:%02dZ new=%04d-%02d-%02dT%02d:%02d:%02dZ",
                 sysBeforeUtc.tm_year + 1900, sysBeforeUtc.tm_mon + 1, sysBeforeUtc.tm_mday,
                 sysBeforeUtc.tm_hour, sysBeforeUtc.tm_min, sysBeforeUtc.tm_sec,
                 rtcTime.year(), rtcTime.month(), rtcTime.day(),
                 rtcTime.hour(), rtcTime.minute(), rtcTime.second());

            unlockRtc();
            
            return true;
        }

        bool restoreSystemTimeOrFallback() {
            if (syncSystemTimeFromRTC()) {
                return true;
            }

            // If RTC invalid, set fallback to keep timestamps monotonic in battery_mode
            struct timeval tv = {.tv_sec = kFallbackUnixUtc, .tv_usec = 0};
            settimeofday(&tv, nullptr);
            LOGW("Fallback: System time set to 2025-01-01 00:00:00 UTC (RTC invalid)");
            return false;
        }
        
        time_t getLastSyncTime() {
            return lastSyncTime;
        }
        
        void updateLastSyncTime() {
            lastSyncTime = time(nullptr);
        }
    }
}
