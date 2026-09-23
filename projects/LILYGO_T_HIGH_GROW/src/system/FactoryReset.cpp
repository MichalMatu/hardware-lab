#include "FactoryReset.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>
#if __has_include(<esp_task_wdt.h>)
#include <esp_task_wdt.h>
#define FACTORY_HAS_WDT 1
#else
#define FACTORY_HAS_WDT 0
#endif
#include "Logging.h"
#include "../config/AppConfig.h"

#undef LOG_TAG
#define LOG_TAG "Factory"

namespace {
constexpr const char *kPrefsNamespace = "sensor_cal";
}

namespace System {

void performFactoryReset() {
    LOGW("Starting factory reset: clearing calibration and formatting FS...");

#if FACTORY_HAS_WDT
    // During long operations (LittleFS format), some code paths call
    // esp_task_wdt_reset() from yield(). If TWDT is active but the current
    // task isn't registered, IDF logs "task not found" errors repeatedly.
    // Simplest fix: disable TWDT entirely for the duration of factory reset.
    // We won't restore it since esp_restart() is called immediately after.
    esp_err_t wdtStatus = esp_task_wdt_status(nullptr);
    if (wdtStatus == ESP_OK || wdtStatus == ESP_ERR_NOT_FOUND) {
        // TWDT is active (either this task is registered or other tasks are)
        esp_task_wdt_deinit();
    }
#endif

        // Heartbeat log every ~2s while we work
        auto logProgress = [](const char* step, uint32_t startMs) {
            uint32_t elapsed = millis() - startMs;
            if ((elapsed / FACTORY::PROGRESS_LOG_INTERVAL_MS) != ((elapsed - 1) / FACTORY::PROGRESS_LOG_INTERVAL_MS)) {
                LOGI("[FactoryReset] %s (t=%lu ms)", step, static_cast<unsigned long>(elapsed));
            }
        };

        uint32_t startMs = millis();

    // Clear calibration preferences
    Preferences prefs;
    if (prefs.begin(kPrefsNamespace, false)) {
        prefs.clear();
        prefs.end();
        LOGI("Preferences cleared");
    } else {
        LOGW("Unable to open prefs namespace");
    }

        logProgress("after prefs", startMs);

    // Format LittleFS (logs/config stored there)
    // Unmount first to avoid "Already Mounted" warnings.
    LittleFS.end();
    if (!LittleFS.begin()) {
        LOGW("LittleFS mount failed before format; attempting format anyway");
    }
    if (LittleFS.format()) {
        LOGI("LittleFS formatted");
    } else {
        LOGE("LittleFS format failed");
    }

        logProgress("after format", startMs);

    LOGW("Restarting...");
    delay(FACTORY::PRE_RESTART_DELAY_MS);
    esp_restart();
}

}  // namespace System
