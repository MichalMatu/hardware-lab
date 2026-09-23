#include "RTCPower.h"
#include "../../config/AppConfig.h"
#include "../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "RTC:Power"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using namespace HW;

namespace RTCModule {
    namespace Power {
        void powerOn() {
            digitalWrite(RTC_POWER, HIGH);
            // Wait for DS3231 to stabilize after power-on
            vTaskDelay(pdMS_TO_TICKS(RTC::POWER_STABILIZE_DELAY_MS));
        }
        
        void powerOff() {
            digitalWrite(RTC_POWER, LOW);
        }
    }
}
