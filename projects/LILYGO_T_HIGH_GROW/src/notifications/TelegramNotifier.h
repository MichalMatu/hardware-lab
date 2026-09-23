#pragma once

#include <Arduino.h>

// Forward declaration
class TelegramSettingsService;

namespace NOTIFY {

struct TelegramSendResult {
    int httpCode{0};
    String error;
    String tlsError;
    String response;
};

// Forward declarations for telegram submodules
namespace TELEGRAM {
    class TelegramConnectionValidator;
    class TelegramTlsConfig;
    class TelegramHttpClient;
}

class TelegramNotifier {
public:
    TelegramNotifier() = default;

    // Initialize with settings service (must be called before use)
    void setSettingsService(TelegramSettingsService* settingsService);

    // Default: uses TLS with disabled certificate validation (setInsecure).
    // Optional: enable cert validation with -DTELEGRAM_TLS_VERIFY=1 (uses setCACert with a pinned Root CA).
    bool sendMessage(const String& text, TelegramSendResult& out);

    bool isConfigured() const;

private:
    TelegramSettingsService* _settingsService = nullptr;
};

}  // namespace NOTIFY
