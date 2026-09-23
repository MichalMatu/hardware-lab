/**
 * @file TelegramTestTask.h
 * @brief Telegram test sender worker (persistent task)
 *
 * Runs Telegram test sends in a single long-lived FreeRTOS task.
 * This avoids per-request task creation/deletion and reduces heap fragmentation.
 */

#ifndef TelegramTestTask_h
#define TelegramTestTask_h

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "../../notifications/TelegramNotifier.h"
#include "../../notifications/telegram/TelegramSettingsService.h"

namespace API {
namespace TelegramTest {

/**
 * Submit a Telegram test send to the persistent worker and wait for completion.
 *
 * Notes:
 * - The HTTP handler may time out; the worker still finishes and will release
 *   the provided concurrencyGuard when done.
 * - This function does not create per-request tasks.
 *
 * @param text Null-terminated message text.
 * @param settingsService Telegram settings service.
 * @param concurrencyGuard Optional guard to release after work completes.
 * @param timeoutMs Max time to wait for send completion.
 * @param out Filled with Telegram send result if completed in time.
 * @return true if the worker completed within timeoutMs, false otherwise.
 */
bool submitAndWait(const char* text,
                   TelegramSettingsService* settingsService,
                   SemaphoreHandle_t concurrencyGuard,
                   uint32_t timeoutMs,
                   NOTIFY::TelegramSendResult& out);

}  // namespace TelegramTest
}  // namespace API

#endif
