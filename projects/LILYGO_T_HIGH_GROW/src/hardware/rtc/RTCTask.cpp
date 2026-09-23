#include "RTCTask.h"
#include "../../config/AppConfig.h"
#include "../../system/Logging.h"
#include "RTCModule.h"
#include <lwip/apps/sntp.h>
#include <time.h>

// IntelliSense/lwIP config sometimes omits sync status definitions; provide a
// minimal fallback for editors without changing runtime logic (guarded by
// __INTELLISENSE__).
#ifdef __INTELLISENSE__
typedef enum {
  SNTP_SYNC_STATUS_RESET = 0,
  SNTP_SYNC_STATUS_COMPLETED = 1,
} sntp_sync_status_t;
#endif

#undef LOG_TAG
#define LOG_TAG "RTCTask"

namespace {
constexpr int kMinValidYear = 2025;
constexpr int kMaxValidYear = 2040;
// 2025-01-01 00:00:00 UTC
constexpr time_t kFallbackUnixUtc = 1735689600;

bool isYearValid(int year) {
  return year >= kMinValidYear && year <= kMaxValidYear;
}

int getLocalYear(time_t t) {
  struct tm timeinfo;
  localtime_r(&t, &timeinfo);
  return timeinfo.tm_year + 1900;
}

bool isSystemTimeValid() {
  time_t now = time(nullptr);
  return isYearValid(getLocalYear(now));
}

// Track last known SNTP sync status to detect new syncs (avoid periodic false
// positives)
#if defined(SNTP_SYNC_STATUS_COMPLETED)
sntp_sync_status_t lastSntpStatus = SNTP_SYNC_STATUS_RESET;
#else
// Fallback tracker for time jumps when sync status API not available
time_t lastSeenTime = 0;
#endif

bool hasSntpJustSynced() {
  if (!sntp_enabled()) {
    return false;
  }

#if defined(SNTP_SYNC_STATUS_COMPLETED)
  // Preferred method: check sync status and trigger only on transition to
  // COMPLETED
  sntp_sync_status_t status = sntp_get_sync_status();
  bool justSynced = (status == SNTP_SYNC_STATUS_COMPLETED) &&
                    (lastSntpStatus != SNTP_SYNC_STATUS_COMPLETED);
  lastSntpStatus = status;
  return justSynced;
#else
  // Fallback: detect time jumps (new sync likely occurred)
  // Compare with last RTC sync to avoid spurious triggers
  time_t currentTime = time(nullptr);
  time_t lastRtcSync = RTCModule::getLastSyncTime();

  // If time jumps by >5s relative to last seen and differs from last RTC sync,
  // assume new SNTP sync
  if (lastSeenTime != 0 &&
      abs(currentTime - lastSeenTime) > RTC::TIME_JUMP_THRESHOLD_SEC &&
      abs(currentTime - lastRtcSync) > RTC::TIME_JUMP_THRESHOLD_SEC) {
    lastSeenTime = currentTime;
    return true;
  }

  // First reasonable time after boot with no RTC sync yet
  if (lastSeenTime == 0 &&
      currentTime > kFallbackUnixUtc + RTC::SYNC_VALID_AFTER_FALLBACK_SEC) {
    lastSeenTime = currentTime;
    return true;
  }

  lastSeenTime = currentTime;
  return false;
#endif
}
} // namespace

// Static members
TaskHandle_t RTCTask::_taskHandle = nullptr;

bool RTCTask::begin() {
  // Initialize RTC module hardware first
  LOGI("Initializing DS3231 module (GPIO 12/13/15)...");
  if (!RTCModule::begin()) {
    LOGE("RTC initialization failed!");
    LOGW("Check hardware connections: Power GPIO12->VCC, SDA GPIO13->RTC SDA, "
         "SCL GPIO15->RTC SCL");
    LOGE("Task NOT started - hardware not available");
    return false;
  }

  // Restore system time from RTC if ESP32 time is invalid (boot recovery)
  time_t sysTime = time(nullptr);
  int year = getLocalYear(sysTime);

  if (!isYearValid(year)) {
    LOGW("System time invalid (year=%d), attempting restore from RTC...", year);
    if (RTCModule::syncSystemTimeFromRTC()) {
      LOGI("System time successfully restored from RTC battery backup");
    } else {
      LOGW("Could not restore time from RTC, using fallback date");
      // Fallback to 2025-01-01 00:00:00 UTC (TZ-safe)
      struct timeval tv = {.tv_sec = kFallbackUnixUtc, .tv_usec = 0};
      settimeofday(&tv, nullptr);
      LOGI("Fallback: System time set to 2025-01-01 00:00:00 UTC");
    }
  } else {
    LOGI("System time already valid (%d), no RTC restore needed", year);
  }

  // Create FreeRTOS task for RTC management
  BaseType_t result =
      xTaskCreatePinnedToCore(taskLoop, "RTCTask",
                              3072, // Stack size for time operations
                              nullptr,
                              1, // Same priority as SensorLoggingTask
                              &_taskHandle,
                              1 // Core 1 (same as other application tasks)
      );

  if (result != pdPASS) {
    LOGE("Failed to create task");
    return false;
  }

  LOGI("Task started - periodic RTC reads and NTP sync enabled");
  return true;
}

void RTCTask::stop() {
  if (_taskHandle != nullptr) {
    vTaskDelete(_taskHandle);
    _taskHandle = nullptr;
    LOGI("Task stopped");
  }
}

void RTCTask::taskLoop(void *parameter) {
  bool firstSyncDone = false;
  uint32_t lastSyncAttempt = 0;

  LOGI("Loop started - RTC writes on NTP sync (max once per minute)");

  while (true) {
    uint32_t now = millis();

    // Production sync policy:
    // - Do NOT write RTC "blindly".
    // - Write RTC only when SNTP reports a completed sync.
    // - First sync attempt gated to >= 30s uptime to avoid early noise.
    // - Throttle syncs to max once per kMinSyncIntervalMs (60s).
    if (now > RTC::FIRST_SYNC_DELAY_MS &&
        (now - lastSyncAttempt) >= RTC::MIN_SYNC_INTERVAL_MS) {
      if (hasSntpJustSynced() && isSystemTimeValid()) {
        if (!firstSyncDone) {
          LOGI("First NTP sync detected - updating RTC");
          firstSyncDone = true;
        } else {
          LOGI("NTP sync event detected - updating RTC");
        }

        if (RTCModule::syncWithSystemTime()) {
          LOGI("RTC synchronized with NTP time");
          lastSyncAttempt = now;
        } else {
          LOGW("Failed to sync RTC with NTP time");
          // Still update lastSyncAttempt to avoid retry spam
          lastSyncAttempt = now;
        }
      }
    }

    // Sleep for 60 seconds (minimal CPU load, sufficient for rare NTP syncs)
    vTaskDelay(pdMS_TO_TICKS(RTC::TASK_SLEEP_MS));
  }
}
