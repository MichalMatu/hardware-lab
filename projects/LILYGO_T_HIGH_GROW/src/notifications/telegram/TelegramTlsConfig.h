#pragma once

#include <NetworkClientSecure.h>

namespace NOTIFY {

struct TelegramSendResult;

namespace TELEGRAM {

class TelegramTlsConfig {
public:
    static void configure(NetworkClientSecure& client);

#if defined(TELEGRAM_TLS_VERIFY) && TELEGRAM_TLS_VERIFY
    static bool configureTlsWithRootCa(NetworkClientSecure& client, TelegramSendResult& out);
#endif
};

}  // namespace TELEGRAM
}  // namespace NOTIFY
