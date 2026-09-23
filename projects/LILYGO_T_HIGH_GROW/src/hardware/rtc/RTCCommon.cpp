#include "RTCCommon.h"
#include "../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "RTC"

namespace RTCModule {
    namespace Internal {
        // Shared RTC hardware instance and state
        RTC_DS3231 rtc;
        bool initialized = false;
        SemaphoreHandle_t rtcMutex = nullptr;
        time_t lastSyncTime = 0;
        
        bool isYearValid(int year) {
            return year >= kMinValidYear && year <= kMaxValidYear;
        }

        bool lockRtc(TickType_t timeoutTicks) {
            if (rtcMutex == nullptr) {
                return true;
            }
            return xSemaphoreTake(rtcMutex, timeoutTicks) == pdTRUE;
        }

        void unlockRtc() {
            if (rtcMutex != nullptr) {
                xSemaphoreGive(rtcMutex);
            }
        }
    }
}
