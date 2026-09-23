/**
 * @file Rf433TransmissionHandler.cpp
 * @brief RF433 transmission handler implementation
 */

#include "Rf433TransmissionHandler.h"

#include <RCSwitch.h>

#include "../../../config/HardwareConfig.h"
#include "../../../config/Rf433Config.h"
#include "../../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "RF433Tx"

namespace {

RCSwitch g_radio;
bool g_initialized = false;

}  // namespace

namespace Rf433TransmissionHandler {

void init() {
    if (g_initialized) {
        return;
    }
    
    g_radio.enableTransmit(HW::RF433_TX);
    g_radio.setRepeatTransmit(RF433::DEFAULT_REPEAT_COUNT);
    g_radio.setProtocol(RF433::DEFAULT_PROTOCOL);
    g_initialized = true;
    
    LOGD("Radio initialized: TX pin=%u repeat=%u protocol=%u",
         static_cast<unsigned>(HW::RF433_TX),
         RF433::DEFAULT_REPEAT_COUNT,
         RF433::DEFAULT_PROTOCOL);
}

bool isReady() {
    return g_initialized;
}

void transmit(const Rf433Internal::TxRequest& request) {
    if (!g_initialized) {
        LOGW("Radio not initialized, cannot transmit");
        return;
    }
    
    // Configure radio for this transmission
    g_radio.setProtocol(request.protocol);
    if (request.pulseLength > 0) {
        g_radio.setPulseLength(request.pulseLength);
    }
    g_radio.setRepeatTransmit(request.repeat);
    
    // Log before transmission
    LOGD("TX start: %s value=0x%08lX bits=%u proto=%u repeat=%u pulse=%u",
         request.logLabel[0] ? request.logLabel : "unnamed",
         static_cast<unsigned long>(request.value),
         static_cast<unsigned>(request.bitLength),
         static_cast<unsigned>(request.protocol),
         static_cast<unsigned>(request.repeat),
         static_cast<unsigned>(request.pulseLength));
    
    // Perform actual transmission (blocking call)
    g_radio.send(request.value, request.bitLength);
    
    // Log completion
    LOGD("TX complete: %s", request.logLabel[0] ? request.logLabel : "unnamed");
}

}  // namespace Rf433TransmissionHandler
