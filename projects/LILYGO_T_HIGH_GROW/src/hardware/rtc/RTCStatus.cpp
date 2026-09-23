#include "RTCStatus.h"
#include "RTCPower.h"
#include "../../system/Logging.h"
#include <ArduinoJson.h>
#include <PsychicHttp.h>

#undef LOG_TAG
#define LOG_TAG "RTC:Status"

using namespace RTCModule::Internal;

namespace RTCModule {
    namespace Status {
        esp_err_t getStatus(PsychicRequest *request) {
            if (!initialized) {
                return request->reply(503, "application/json", 
                    "{\"status\":\"error\",\"message\":\"RTC module not initialized\"}");
            }
            
            if (!lockRtc()) {
                return request->reply(503, "application/json", 
                    "{\"status\":\"error\",\"message\":\"RTC mutex timeout\"}");
            }
            
            // Power on RTC
            Power::powerOn();
            
            // Read current time from RTC
            DateTime rtcTime = rtc.now();
            bool lostPower = rtc.lostPower();
            
            // Power off
            Power::powerOff();
            
            unlockRtc();
            
            // Build JSON response
            // NOTE: This project uses ArduinoJson v7+. In v7, `StaticJsonDocument` is only a
            // deprecated compatibility alias and DOES NOT provide a fixed-size stack buffer.
            // Use `JsonDocument` here (same behavior) and keep responses small to reduce heap
            // pressure, or switch to a custom fixed arena allocator if/when we need hard bounds.
            JsonDocument doc;
            doc["initialized"] = true;
            doc["lost_power"] = lostPower;
            
            // RTC time in local timezone (consistent with NTPStatus local_time format)
            // RTC stores UTC internally, convert to local time for display
            time_t rtcUnixTime = rtcTime.unixtime();
            struct tm rtcLocalTime;
            localtime_r(&rtcUnixTime, &rtcLocalTime);
            char timeStr[32];
            snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02dT%02d:%02d:%02d",
                     rtcLocalTime.tm_year + 1900, rtcLocalTime.tm_mon + 1, rtcLocalTime.tm_mday,
                     rtcLocalTime.tm_hour, rtcLocalTime.tm_min, rtcLocalTime.tm_sec);
            doc["rtc_time"] = timeStr;
            doc["rtc_unix"] = rtcUnixTime;
            
            // Last sync info in local timezone (consistent with NTPStatus format)
            if (lastSyncTime > 0) {
                doc["last_sync_unix"] = lastSyncTime;
                struct tm syncTimeinfo;
                localtime_r(&lastSyncTime, &syncTimeinfo);
                char syncStr[32];
                snprintf(syncStr, sizeof(syncStr), "%04d-%02d-%02dT%02d:%02d:%02d",
                         syncTimeinfo.tm_year + 1900, syncTimeinfo.tm_mon + 1, syncTimeinfo.tm_mday,
                         syncTimeinfo.tm_hour, syncTimeinfo.tm_min, syncTimeinfo.tm_sec);
                doc["last_sync_time"] = syncStr;
            } else {
                doc["last_sync_unix"] = 0;
                doc["last_sync_time"] = "never";
            }
            
            // Serialize and send
            String response;
            serializeJson(doc, response);
            return request->reply(200, "application/json", response.c_str());
        }
    }
}
