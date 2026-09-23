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

#include <wifi/APSettingsService.h>
#include <services/RestartService.h>
#include <ESPmDNS.h>

APSettingsService::APSettingsService(PsychicHttpServer *server,
                                     FS *fs,
                                     SecurityManager *securityManager) : _server(server),
                                                                         _securityManager(securityManager),
                                                                         _httpEndpoint(APSettings::read, APSettings::update, this, server, AP_SETTINGS_SERVICE_PATH, securityManager),
                                                                         _fsPersistence(APSettings::read, APSettings::update, this, fs, AP_SETTINGS_FILE),
                                                                         _dnsServer(nullptr)
{
    addUpdateHandler([&](const String &originId)
                     {
                         // HttpEndpoint triggers update handlers BEFORE sending the HTTP response.
                         // Only schedule here; restart happens from loop() so response can be sent.
                         scheduleRestart();
                     },
                     false);
}

void APSettingsService::scheduleRestart(unsigned long delayMs)
{
    _restartPending = true;
    _restartAt = millis() + delayMs;
    ESP_LOGI(SVK_TAG, "AP settings changed. Scheduling hard restart in %lu ms", delayMs);
}

void APSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}

void APSettingsService::loop()
{
    unsigned long currentMillis = millis();

    if (_restartPending && (long)(currentMillis - _restartAt) >= 0)
    {
        _restartPending = false;
        RestartService::restartNow();
        return;
    }

    handleDNS();
}

void APSettingsService::forceAPMode()
{
    if (!_apStarted)
    {
        // NOTE: WiFiSettingsService may call forceAPMode() very early during boot
        // (from initWiFi()->manageSTA()) before APSettingsService::begin() has
        // had a chance to load defaults from FS. After a factory reset, this can
        // leave SSID/password unset, preventing the AP from starting.
        auto isZeroIp = [](const IPAddress &ip) {
            return ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0;
        };

        if (_state.ssid.isEmpty() || isZeroIp(_state.localIP))
        {
            _fsPersistence.readFromFS();

            // Defensive defaults in case FS isn't writable yet.
            if (_state.ssid.isEmpty())
            {
                _state.ssid = SettingValue::format(FACTORY_AP_SSID);
            }
            if (_state.password.isEmpty())
            {
                _state.password = FACTORY_AP_PASSWORD;
            }
            if (_state.channel == 0)
            {
                _state.channel = FACTORY_AP_CHANNEL;
            }
            if (_state.maxClients == 0)
            {
                _state.maxClients = FACTORY_AP_MAX_CLIENTS;
            }
            if (isZeroIp(_state.localIP))
            {
                _state.localIP.fromString(FACTORY_AP_LOCAL_IP);
            }
            if (isZeroIp(_state.gatewayIP))
            {
                _state.gatewayIP.fromString(FACTORY_AP_GATEWAY_IP);
            }
            if (isZeroIp(_state.subnetMask))
            {
                _state.subnetMask.fromString(FACTORY_AP_SUBNET_MASK);
            }
        }

        ESP_LOGI(SVK_TAG, "Forcing AP mode.");
        startAP();
        _apStarted = true;
    }
}

void APSettingsService::startAP()
{
    ESP_LOGI(SVK_TAG, "Starting software access point (ssid=%s)", _state.ssid.c_str());
    // CRITICAL: Force pure AP mode, disable STA completely
    // Stop mDNS to avoid IGMP assertions when switching to AP-only
    MDNS.end();
    WiFi.disconnect(true);  // Disconnect and disable STA
    WiFi.mode(WIFI_AP);     // Set mode to AP ONLY, not AP_STA

    bool okConfig = WiFi.softAPConfig(_state.localIP, _state.gatewayIP, _state.subnetMask);
    const char *passwordArg = nullptr;
    if (!_state.password.isEmpty())
    {
        passwordArg = _state.password.c_str();
    }

    bool okAp = WiFi.softAP(_state.ssid.c_str(), passwordArg, _state.channel, _state.ssidHidden, _state.maxClients);

    if (!okConfig || !okAp)
    {
        ESP_LOGE(SVK_TAG,
                 "Failed to start AP (softAPConfig=%d, softAP=%d, ssid_len=%u, channel=%u)",
                 okConfig ? 1 : 0,
                 okAp ? 1 : 0,
                 (unsigned)_state.ssid.length(),
                 (unsigned)_state.channel);
    }
#if CONFIG_IDF_TARGET_ESP32C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // https://www.wemos.cc/en/latest/c3/c3_mini_1_0_0.html#about-wifi
#endif
    if (!_dnsServer)
    {
        IPAddress apIp = WiFi.softAPIP();
#ifdef SERIAL_INFO
        ESP_LOGI(SVK_TAG, "Starting captive portal on %s", apIp.toString().c_str());
#endif
        _dnsServer = new DNSServer;
        _dnsServer->start(DNS_PORT, "*", apIp);
    }
}

void APSettingsService::stopAP()
{
    if (_dnsServer)
    {
        ESP_LOGI(SVK_TAG, "Stopping captive portal");
        _dnsServer->stop();
        delete _dnsServer;
        _dnsServer = nullptr;
    }
    ESP_LOGI(SVK_TAG, "Stopping software access point");
    WiFi.softAPdisconnect(true);
}

void APSettingsService::handleDNS()
{
    if (_dnsServer)
    {
        _dnsServer->processNextRequest();
    }
}

APNetworkStatus APSettingsService::getAPNetworkStatus()
{
    WiFiMode_t currentWiFiMode = WiFi.getMode();
    bool apActive = currentWiFiMode == WIFI_AP;
    return apActive ? APNetworkStatus::ACTIVE : APNetworkStatus::INACTIVE;
}
