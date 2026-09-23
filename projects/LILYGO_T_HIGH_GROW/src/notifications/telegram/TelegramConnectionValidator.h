#pragma once

#include <Arduino.h>

namespace NOTIFY {
namespace TELEGRAM {

class TelegramConnectionValidator {
public:
    static bool ensureOnline(String& errorOut, uint32_t ntpWaitMs);

private:
    static bool isSystemTimeValid();
    static bool isYearValid(int year);

    static constexpr int kMinValidYear = 2020;
    static constexpr int kMaxValidYear = 2099;
};

}  // namespace TELEGRAM
}  // namespace NOTIFY
