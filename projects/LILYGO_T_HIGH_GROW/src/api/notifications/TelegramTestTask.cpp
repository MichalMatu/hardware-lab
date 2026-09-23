/**
 * @file TelegramTestTask.cpp
 * @brief Implementation of async Telegram test task
 */

#include "TelegramTestTask.h"
#include "../../config/AppConfig.h"
#include "../../system/Logging.h"

#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#undef LOG_TAG
#define LOG_TAG "TelegramTask"

namespace API {
namespace TelegramTest {

namespace {
constexpr uint32_t kWorkerStackBytes = 8 * 1024;
constexpr uint32_t kWorkerStackWords = kWorkerStackBytes / sizeof(StackType_t);
constexpr UBaseType_t kWorkerPriority = 5;
constexpr BaseType_t kWorkerCore = 1;

TaskHandle_t gWorkerTask = nullptr;

// Semaphores and buffers are static to avoid heap churn.
SemaphoreHandle_t gRequestSem = nullptr;
SemaphoreHandle_t gDoneSem = nullptr;
SemaphoreHandle_t gCleanupSem = nullptr;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
StaticSemaphore_t gRequestSemBuf;
StaticSemaphore_t gDoneSemBuf;
StaticSemaphore_t gCleanupSemBuf;

StaticTask_t gWorkerTaskBuf;
StackType_t gWorkerStack[kWorkerStackWords];
#endif

TelegramSettingsService* gSettingsService = nullptr;
SemaphoreHandle_t gConcurrencyGuard = nullptr;

char gTextBuf[APP::NOTIFY::TELEGRAM_MAX_TEXT_LEN + 1] = {0};
NOTIFY::TelegramSendResult gResult;

void drainBinarySemaphore(SemaphoreHandle_t sem) {
    if (!sem) return;
    while (xSemaphoreTake(sem, 0) == pdTRUE) {
        // drain
    }
}

void workerLoop(void* /*param*/) {
    for (;;) {
        // Wait for a request.
        (void)xSemaphoreTake(gRequestSem, portMAX_DELAY);

        LOGI("Worker woke on core %d", xPortGetCoreID());
        LOGI("HEAP: free=%u maxAlloc=%u", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

        // Snapshot request state (single-producer due to endpoint guard).
        TelegramSettingsService* settings = gSettingsService;
        SemaphoreHandle_t guard = gConcurrencyGuard;

        // Perform the actual send with TLS/HTTPS.
        gResult = NOTIFY::TelegramSendResult{};
        NOTIFY::TelegramNotifier notifier;
        notifier.setSettingsService(settings);
        const bool ok = notifier.sendMessage(String(gTextBuf), gResult);

        LOGI("sendMessage returned: ok=%d httpCode=%d", ok ? 1 : 0, gResult.httpCode);
        LOGI("HEAP AFTER: free=%u maxAlloc=%u", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

        // Signal completion to HTTP handler.
        xSemaphoreGive(gDoneSem);

        // Allow handler to acknowledge it copied the result.
        (void)xSemaphoreTake(gCleanupSem, pdMS_TO_TICKS(500));

        // Release concurrency guard (allows next test) even if handler timed out.
        if (guard != nullptr) {
            xSemaphoreGive(guard);
        }
    }
}

bool ensureWorkerStarted() {
    if (gWorkerTask != nullptr) {
        return true;
    }

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (!gRequestSem) {
        gRequestSem = xSemaphoreCreateBinaryStatic(&gRequestSemBuf);
    }
    if (!gDoneSem) {
        gDoneSem = xSemaphoreCreateBinaryStatic(&gDoneSemBuf);
    }
    if (!gCleanupSem) {
        gCleanupSem = xSemaphoreCreateBinaryStatic(&gCleanupSemBuf);
    }
#else
    if (!gRequestSem) {
        gRequestSem = xSemaphoreCreateBinary();
    }
    if (!gDoneSem) {
        gDoneSem = xSemaphoreCreateBinary();
    }
    if (!gCleanupSem) {
        gCleanupSem = xSemaphoreCreateBinary();
    }
#endif

    if (!gRequestSem || !gDoneSem || !gCleanupSem) {
        LOGW("Failed to create Telegram worker semaphores");
        return false;
    }

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    gWorkerTask = xTaskCreateStaticPinnedToCore(
        workerLoop,
        "telegram_worker",
        kWorkerStackWords,
        nullptr,
        kWorkerPriority,
        gWorkerStack,
        &gWorkerTaskBuf,
        kWorkerCore);
#else
    BaseType_t ok = xTaskCreatePinnedToCore(
        workerLoop,
        "telegram_worker",
        kWorkerStackWords,
        nullptr,
        kWorkerPriority,
        &gWorkerTask,
        kWorkerCore);
    if (ok != pdPASS) {
        gWorkerTask = nullptr;
    }
#endif

    if (gWorkerTask == nullptr) {
        LOGW("Failed to create Telegram worker task");
        return false;
    }

    LOGI("Telegram worker started (stack=%uB)", kWorkerStackBytes);
    return true;
}
} // namespace

bool submitAndWait(const char* text,
                   TelegramSettingsService* settingsService,
                   SemaphoreHandle_t concurrencyGuard,
                   uint32_t timeoutMs,
                   NOTIFY::TelegramSendResult& out) {
    out = NOTIFY::TelegramSendResult{};

    if (!text || text[0] == '\0' || settingsService == nullptr) {
        return false;
    }

    if (!ensureWorkerStarted()) {
        return false;
    }

    // Prepare request (single producer due to API guard semaphore).
    const size_t maxLen = APP::NOTIFY::TELEGRAM_MAX_TEXT_LEN;
    strncpy(gTextBuf, text, maxLen);
    gTextBuf[maxLen] = '\0';
    gSettingsService = settingsService;
    gConcurrencyGuard = concurrencyGuard;

    // Ensure semaphores are in a known state.
    drainBinarySemaphore(gDoneSem);
    drainBinarySemaphore(gCleanupSem);

    // Kick worker.
    xSemaphoreGive(gRequestSem);

    // Wait for completion.
    const TickType_t timeoutTicks = pdMS_TO_TICKS(timeoutMs);
    if (xSemaphoreTake(gDoneSem, timeoutTicks) != pdTRUE) {
        // Worker will still complete and release concurrencyGuard.
        return false;
    }

    out = gResult;
    xSemaphoreGive(gCleanupSem);
    return true;
}

}  // namespace TelegramTest
}  // namespace API
