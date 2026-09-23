#ifndef ESP32SvelteKit_h
#define ESP32SvelteKit_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Arduino.h>

#include <WiFi.h>
#include <ESPmDNS.h>
#include <services/FeaturesService.h>
#include <wifi/APSettingsService.h>
#include <wifi/APStatus.h>
#include <security/AuthenticationService.h>
#include <services/BatteryService.h>
#include <services/FactoryResetService.h>
#include <core/EventSocket.h>
#include <services/NotificationService.h>
#include <network/NTPSettingsService.h>
#include <network/NTPStatus.h>
#include <services/RestartService.h>
#include <security/SecuritySettingsService.h>
#include <services/SleepService.h>
#include <system/SystemStatus.h>
#include <wifi/WiFiScanner.h>
#include <wifi/WiFiSettingsService.h>
#include <wifi/WiFiStatus.h>
#include <core/ESPFS.h>
#include <PsychicHttp.h>
#include <vector>

#ifdef EMBED_WWW
#include <core/WWWData.h>
#endif

#ifndef CORS_ORIGIN
#define CORS_ORIGIN "*"
#endif

#ifndef APP_VERSION
#define APP_VERSION "demo"
#endif

#ifndef APP_NAME
#define APP_NAME "ESP32 SvelteKit Demo"
#endif

#ifndef ESP32SVELTEKIT_RUNNING_CORE
#define ESP32SVELTEKIT_RUNNING_CORE -1
#endif

#ifndef ESP32SVELTEKIT_LOOP_INTERVAL
#define ESP32SVELTEKIT_LOOP_INTERVAL 10
#endif

// define callback function to include into the main loop
typedef std::function<void()> loopCallback;

// enum for connection status
enum class ConnectionStatus
{
    OFFLINE,
    AP,
    AP_CONNECTED,
    STA,
    STA_CONNECTED
};

class ESP32SvelteKit
{
public:
    ESP32SvelteKit(PsychicHttpServer *server, unsigned int numberEndpoints = 115);

    void begin();

    ConnectionStatus getConnectionStatus()
    {
        return _connectionStatus;
    }

    FS *getFS()
    {
        return &ESPFS;
    }

    PsychicHttpServer *getServer()
    {
        return _server;
    }

    SecurityManager *getSecurityManager()
    {
        return &_securitySettingsService;
    }

    EventSocket *getSocket()
    {
        return &_socket;
    }

#if FT_ENABLED(FT_SECURITY)
    SecuritySettingsService *getSecuritySettingsService()
    {
        return &_securitySettingsService;
    }
#endif

    WiFiSettingsService *getWiFiSettingsService()
    {
        return &_wifiSettingsService;
    }

    APSettingsService *getAPSettingsService()
    {
        return &_apSettingsService;
    }

    NotificationService *getNotificationService()
    {
        return &_notificationService;
    }

#if FT_ENABLED(FT_NTP)
    NTPSettingsService *getNTPSettingsService()
    {
        return &_ntpSettingsService;
    }
#endif


#if FT_ENABLED(FT_SLEEP)
    SleepService *getSleepService()
    {
        return &_sleepService;
    }
#endif

#if FT_ENABLED(FT_BATTERY)
    BatteryService *getBatteryService()
    {
        return &_batteryService;
    }
#endif

    FeaturesService *getFeatureService()
    {
        return &_featureService;
    }

    RestartService *getRestartService()
    {
        return &_restartService;
    }

    void factoryReset()
    {
        _factoryResetService.factoryReset();
    }

    void setMDNSAppName(String name)
    {
        _appName = name;
    }

    void addLoopFunction(loopCallback function)
    {
        _loopFunctions.push_back(function);
    }

private:
    PsychicHttpServer *_server;
    TaskHandle_t _loopTaskHandle;
    unsigned int _numberEndpoints;
    FeaturesService _featureService;
    SecuritySettingsService _securitySettingsService;
    WiFiSettingsService _wifiSettingsService;
    WiFiScanner _wifiScanner;
    WiFiStatus _wifiStatus;
    APSettingsService _apSettingsService;
    APStatus _apStatus;
    EventSocket _socket;
    NotificationService _notificationService;
#if FT_ENABLED(FT_NTP)
    NTPSettingsService _ntpSettingsService;
    NTPStatus _ntpStatus;
#endif
#if FT_ENABLED(FT_SECURITY)
    AuthenticationService _authenticationService;
#endif
#if FT_ENABLED(FT_SLEEP)
    SleepService _sleepService;
#endif
#if FT_ENABLED(FT_BATTERY)
    BatteryService _batteryService;
#endif
    RestartService _restartService;
    FactoryResetService _factoryResetService;
    SystemStatus _systemStatus;

    String _appName = APP_NAME;

protected:
    static void _loopImpl(void *_this) { static_cast<ESP32SvelteKit *>(_this)->_loop(); }
    void _loop();

    std::vector<loopCallback> _loopFunctions;

    // Connectivity status
    ConnectionStatus _connectionStatus = ConnectionStatus::OFFLINE;
};

#endif
