/**
 * @file NotificationsConfig.h
 * @brief Notifications (Telegram) configuration
 * 
 * Telegram API timeouts, payload limits, and connection parameters.
 */

#ifndef NotificationsConfig_h
#define NotificationsConfig_h

#include <Arduino.h>

// ============================================================================
// Notifications (Telegram)
// ============================================================================
namespace APP {
namespace NOTIFY {
    // Hard cap to protect RAM and avoid accidental huge payloads
    constexpr size_t TELEGRAM_MAX_TEXT_LEN = 1024;

    // Max time to wait for the system time to become valid (e.g. after NTP)
    constexpr uint32_t TELEGRAM_TIME_WAIT_MS = 5000;

    // TCP probe timeout for internet reachability (api.telegram.org:443)
    constexpr uint32_t TELEGRAM_ONLINE_PROBE_TIMEOUT_MS = 2000;

    // Socket read/write timeout for Telegram HTTPS request/response.
    // This limits how long POST/getString can block after TLS is established.
    constexpr uint32_t TELEGRAM_HTTP_IO_TIMEOUT_MS = 10000;  // Reduced from 15s
}
}

#endif
