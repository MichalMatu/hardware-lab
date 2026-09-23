/**
 * @file AppConfig.h
 * @brief Central configuration aggregator for all firmware constants and timeouts
 * 
 * This file includes all domain-specific configuration headers for backward
 * compatibility. Existing code can include AppConfig.h and access all namespaces.
 * 
 * For new code, consider including specific domain config headers directly:
 * - HardwareConfig.h - GPIO pins and hardware addresses (HW, COM)
 * - SensorTimingConfig.h - Sensor timing, logging, button (SENSOR, LOGGER, BTN)
 * - NetworkConfig.h - Network and API timeouts (NET, API)
 * - RTCConfig.h - RTC and time sync (RTC)
 * - NotificationsConfig.h - Telegram settings (APP::NOTIFY)
 * - PowerConfig.h - Power management (POWER)
 * - SystemConfig.h - Application metadata and factory reset (APP, FACTORY)
 * 
 * Modifying values in any config file affects firmware behavior - always test after changes.
 */

#ifndef AppConfig_h
#define AppConfig_h

// Include all domain-specific configuration headers
#include "HardwareConfig.h"
#include "SensorTimingConfig.h"
#include "NetworkConfig.h"
#include "RTCConfig.h"
#include "NotificationsConfig.h"
#include "PowerConfig.h"
#include "SystemConfig.h"

#endif
