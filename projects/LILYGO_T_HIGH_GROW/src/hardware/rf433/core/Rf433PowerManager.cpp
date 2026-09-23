/**
 * @file Rf433PowerManager.cpp
 * @brief RF433 power control implementation
 */

#include "Rf433PowerManager.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "../../../config/HardwareConfig.h"
#include "../../../config/Rf433Config.h"
#include "../../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "RF433Pwr"

namespace {

bool g_initialized = false;
bool g_isPoweredOn = false;

}  // namespace

namespace Rf433PowerManager {

void init() {
    if (g_initialized) {
        return;
    }
    
    pinMode(HW::RF433_POWER, OUTPUT);
    digitalWrite(HW::RF433_POWER, LOW);
    g_isPoweredOn = false;
    g_initialized = true;
    
    LOGD("Power manager initialized (pin=%u)", static_cast<unsigned>(HW::RF433_POWER));
}

bool isInitialized() {
    return g_initialized;
}

void powerOn() {
    if (!g_initialized) {
        init();
    }
    
    if (g_isPoweredOn) {
        return;  // Already on, no-op
    }
    
    digitalWrite(HW::RF433_POWER, HIGH);
    g_isPoweredOn = true;
    
    // Wait for voltage stabilization
    vTaskDelay(pdMS_TO_TICKS(RF433::POWER_ON_STABILIZATION_MS));
    
    LOGD("Power ON (stabilization=%ums)", static_cast<unsigned>(RF433::POWER_ON_STABILIZATION_MS));
}

void powerOff() {
    if (!g_initialized || !g_isPoweredOn) {
        return;  // Not initialized or already off
    }
    
    // Small delay to ensure transmission completed
    vTaskDelay(pdMS_TO_TICKS(RF433::POWER_OFF_DELAY_MS));
    
    digitalWrite(HW::RF433_POWER, LOW);
    g_isPoweredOn = false;
    
    LOGD("Power OFF (delay=%ums)", static_cast<unsigned>(RF433::POWER_OFF_DELAY_MS));
}

}  // namespace Rf433PowerManager
