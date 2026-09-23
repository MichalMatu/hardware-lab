#include "RTCCore.h"
#include "RTCPower.h"
#include "../../config/AppConfig.h"
#include "../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "RTC:Core"

using namespace HW;
using namespace RTCModule::Internal;

namespace RTCModule {
    namespace Core {
        bool begin() {
            LOGI("Initializing DS3231 module...");

            if (rtcMutex == nullptr) {
                rtcMutex = xSemaphoreCreateMutex();
                if (rtcMutex == nullptr) {
                    LOGE("Failed to create mutex (low memory)");
                    return false;
                }
            }
            
            // Setup power control pin and power ON first
            pinMode(RTC_POWER, OUTPUT);
            digitalWrite(RTC_POWER, HIGH);
            LOGI("Power control on GPIO %d -> HIGH (RTC powered)", RTC_POWER);
            vTaskDelay(pdMS_TO_TICKS(150));  // Extra time for DS3231 to stabilize with power
            
            // Configure custom pins BEFORE rtc.begin() initializes Wire1
            Wire1.setPins(RTC_SDA, RTC_SCL);
            LOGI("Wire1 pins configured: SDA=%d, SCL=%d", RTC_SDA, RTC_SCL);
            // NOTE: If pull-ups for SDA/SCL are tied to 3V3 while RTC_POWER is switched,
            // ensure hardware prevents backfeeding the DS3231 through I2C lines.
            
            if (!lockRtc(pdMS_TO_TICKS(RTC::INIT_MUTEX_TIMEOUT_MS))) {
                LOGE("Mutex timeout during init");
                digitalWrite(RTC_POWER, LOW);
                return false;
            }

            // Let RTClib initialize Wire1 with our custom pins
            if (!rtc.begin(&Wire1)) {
                unlockRtc();
                LOGE("RTClib failed to initialize DS3231");
                digitalWrite(RTC_POWER, LOW);  // Power off on failure
                return false;
            }
            
            LOGI("DS3231 found and responding (Wire1 @ 100kHz)");
            
            // Check if RTC lost power (battery dead or first use)
            if (rtc.lostPower()) {
                LOGW("RTC lost power, time may be invalid");
                LOGW("Set time via NTP or manually when WiFi available");

                // Keep RTC in a known-valid range until NTP can correct it.
                rtc.adjust(DateTime(kFallbackUnixUtc));
                LOGI("Fallback: Set to 2025-01-01 00:00:00 UTC");
            }
            
            // Read current time to verify communication
            DateTime now = rtc.now();
            LOGI("Current time (UTC): %04d-%02d-%02d %02d:%02d:%02d",
                 now.year(), now.month(), now.day(),
                 now.hour(), now.minute(), now.second());

            unlockRtc();
            
            // Power off after successful init (Wire1 stays active)
            digitalWrite(RTC_POWER, LOW);
            
            initialized = true;
            LOGI("Initialization complete (power OFF, Wire1 active)");
            
            return true;
        }
        
        void readAndPrint() {
            if (!initialized) {
                LOGE("Module not initialized, call begin() first");
                return;
            }

            if (!lockRtc()) {
                LOGE("Mutex timeout in readAndPrint");
                return;
            }
            
            // Power on RTC
            Power::powerOn();
            
            // Read current time (Wire1 already initialized)
            DateTime now = rtc.now();
            
            // Print formatted timestamp (ISO8601-like) - RTC stores UTC
            LOGI("%04d-%02d-%02d %02d:%02d:%02d UTC",
                 now.year(), now.month(), now.day(),
                 now.hour(), now.minute(), now.second());
            
            // Additional info: Unix timestamp and day of week
            const char* daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
            LOGD("Unix: %lu, Day: %s",
                 now.unixtime(),
                 daysOfWeek[now.dayOfTheWeek()]);
            
            // Power off (Wire1 stays active for next read)
            Power::powerOff();

            unlockRtc();
        }
        
        bool setTimeInternal(time_t unixTime) {
            // Power on RTC
            Power::powerOn();
            
            // Set time (Wire1 already initialized)
            rtc.adjust(DateTime(unixTime));
            
            // Power off (Wire1 stays active)
            Power::powerOff();
            
            return true;
        }
        
        bool setTime(time_t unixTime) {
            if (!initialized) {
                LOGE("Module not initialized");
                return false;
            }

            // Lock with longer timeout for write operations
            if (!lockRtc(pdMS_TO_TICKS(RTC::WRITE_MUTEX_TIMEOUT_MS))) {
                LOGE("Mutex timeout in setTime (waited 2s)");
                return false;
            }
            
            bool result = setTimeInternal(unixTime);

            unlockRtc();
            
            return result;
        }
    }
}
