#ifndef HARDWARE_RTC_STATUS_H
#define HARDWARE_RTC_STATUS_H

#include "RTCCommon.h"

// Forward declaration
class PsychicRequest;

namespace RTCModule {
    namespace Status {
        /**
         * @brief Get RTC status (HTTP endpoint handler)
         * 
         * Returns JSON with RTC time, lost power status, and last sync timestamp.
         * 
         * @param request HTTP request object
         * @return esp_err_t HTTP response status
         */
        esp_err_t getStatus(PsychicRequest *request);
    }
}

#endif
