#include "TelegramConnectionValidator.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include "../../config/AppConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace NOTIFY {
namespace TELEGRAM {

static const char* kTelegramHost = "api.telegram.org";

bool TelegramConnectionValidator::isYearValid(int year) {
    return year >= kMinValidYear && year <= kMaxValidYear;
}

bool TelegramConnectionValidator::isSystemTimeValid() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    return isYearValid(timeinfo.tm_year + 1900);
}

bool TelegramConnectionValidator::ensureOnline(String& errorOut, uint32_t ntpWaitMs) {
    const wifi_mode_t mode = WiFi.getMode();
    if (mode == WIFI_OFF) {
        errorOut = "offline/wifi_off";
        return false;
    }

    if (!WiFi.isConnected()) {
        errorOut = "offline/wifi_not_connected";
        return false;
    }

    // Hard policy: do not send notifications unless we can confirm internet reachability.
    // We use a fast DNS + TCP connect probe to api.telegram.org:443.
    IPAddress resolved;
    bool dnsOk = false;
    for (int attempt = 0; attempt < 2; attempt++) {
        if (WiFi.hostByName(kTelegramHost, resolved)) {
            dnsOk = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    if (!dnsOk) {
        errorOut = "offline/dns_failed";
        return false;
    }

    bool tcpOk = false;
    for (int attempt = 0; attempt < 2; attempt++) {
        WiFiClient probe;
        // WiFiClient::setTimeout is milliseconds in this core.
        probe.setTimeout(APP::NOTIFY::TELEGRAM_ONLINE_PROBE_TIMEOUT_MS);
        if (probe.connect(resolved, 443)) {
            tcpOk = true;
            probe.stop();
            break;
        }
        probe.stop();
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    if (!tcpOk) {
        errorOut = "offline/tcp_connect_failed";
        return false;
    }

    // Also require a valid system time window (runtime policy).
    // We don't rely on SNTP sync-status APIs here because they may be unavailable.
    if (!isSystemTimeValid()) {
        const uint32_t start = millis();
        while ((millis() - start) < ntpWaitMs) {
            if (isSystemTimeValid()) {
                return true;
            }
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        errorOut = "offline/time_invalid";
        return false;
    }

    return true;
}

}  // namespace TELEGRAM
}  // namespace NOTIFY
