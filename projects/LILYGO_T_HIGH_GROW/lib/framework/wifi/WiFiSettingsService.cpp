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

#include <wifi/WiFiSettingsService.h>

#include <ArduinoJson.h>

#if __has_include(<power/PowerManager.h>)
#include <power/PowerManager.h>
#define WIFI_HAS_POWER_MANAGER 1
#else
#define WIFI_HAS_POWER_MANAGER 0
#endif
#include <services/RestartService.h>
#include <ESPmDNS.h>

namespace {
    // Guard window after WiFi.begin() to avoid reconfiguring while the stack is busy.
    // Reduced to 5s for faster failover when network is unavailable or credentials are wrong.
    constexpr uint32_t WIFI_CONNECT_GUARD_MS = 5000;

    void delayedRestartTask(void *param) {
        (void)param;
        vTaskDelay(pdMS_TO_TICKS(DELAYED_RECONNECT_MS));
        RestartService::restartNow();
        vTaskDelete(nullptr);
    }
}

WiFiSettingsService::WiFiSettingsService(PsychicHttpServer *server,
                                         FS *fs,
                                         SecurityManager *securityManager,
                                         EventSocket *socket) : _server(server),
                                                                _securityManager(securityManager),
                                                                _httpEndpoint(WiFiSettings::read, WiFiSettings::update, this, server, WIFI_SETTINGS_SERVICE_PATH, securityManager,
                                                                              AuthenticationPredicates::IS_ADMIN),
                                                                _fsPersistence(WiFiSettings::read, WiFiSettings::update, this, fs, WIFI_SETTINGS_FILE),
                                                                _socket(socket),
                                                                _lastConnectionAttempt(0)
{
    addUpdateHandler([this](const String &originId)
                     {
                         (void)originId;
                         ESP_LOGI(SVK_TAG, "WiFi settings changed. Reconnecting shortly...");
                         delayedReconnect();
                     },
                     false);
}

void WiFiSettingsService::setAPSettingsService(APSettingsService *apSettingsService)
{
    _apSettingsService = apSettingsService;
}

void WiFiSettingsService::initWiFi()
{
    WiFi.mode(WIFI_MODE_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    _fsPersistence.readFromFS();
    
    // Immediate first connection attempt (don't wait for loop delay)
    manageSTA();
}

void WiFiSettingsService::begin()
{
    if (_socket)
    {
        _socket->registerEvent(EVENT_RECONNECT);
    }
    _httpEndpoint.begin();
}

void WiFiSettingsService::delayedReconnect()
{
    // Emit a UI notification event and restart shortly after, without blocking the HTTP handler.
    if (_socket)
    {
        JsonDocument doc;
        doc["delay_ms"] = DELAYED_RECONNECT_MS;
        JsonObject jsonObject = doc.as<JsonObject>();
        _socket->emitEvent(EVENT_RECONNECT, jsonObject);
    }

    if (_delayedRestartTask)
    {
        // Already scheduled
        return;
    }

    xTaskCreatePinnedToCore(delayedRestartTask, "wifi_reconnect", 2048, this, 1, &_delayedRestartTask, 1);
}

void WiFiSettingsService::loop()
{
    unsigned long currentMillis = millis();
    if (!_lastConnectionAttempt || (unsigned long)(currentMillis - _lastConnectionAttempt) >= WIFI_RECONNECTION_DELAY)
    {
        _lastConnectionAttempt = currentMillis;
        manageSTA();
    }
}

String WiFiSettingsService::getHostname()
{
    return _state.hostname;
}

String WiFiSettingsService::getIP()
{
    if (WiFi.isConnected())
    {
        return WiFi.localIP().toString();
    }
    return "Not connected";
}

void WiFiSettingsService::manageSTA()
{
    // If AP mode is active, do not attempt STA
    if (_apModeActive)
    {
        return;
    }

    // Clear busy marker once connected
    if (WiFi.isConnected())
    {
        _connectingSince = 0;
    }

    // If STA mode disabled (offline), switch to AP mode for settings access
    if (_state.staConnectionMode == (u_int8_t)STAConnectionMode::OFFLINE)
    {
        ESP_LOGI(SVK_TAG, "STA mode disabled. Switching to AP mode.");
        switchToAPMode();
        return;
    }

    // If no networks configured, switch to AP mode immediately
    if (_state.wifiSettings.empty())
    {
        ESP_LOGI(SVK_TAG, "No STA networks configured. Switching to AP mode.");
        switchToAPMode();
        return;
    }

    // If already connected, do nothing
    if (WiFi.isConnected())
    {
        return;
    }

    // Avoid re-entering WiFi.begin() while the stack is still handling the previous attempt
    if (_connectingSince != 0 && (unsigned long)(millis() - _connectingSince) < WIFI_CONNECT_GUARD_MS)
    {
        return;
    }

    // Don't interrupt if WiFi is connecting - only try when fully disconnected
    wl_status_t status = WiFi.status();
    if (status != WL_DISCONNECTED && status != WL_NO_SSID_AVAIL && status != WL_CONNECT_FAILED)
    {
        return; // Connection in progress, wait
    }

    // Try to connect
    ESP_LOGI(SVK_TAG, "Connecting to WiFi...");
    connectToWiFi();
}

void WiFiSettingsService::connectToWiFi()
{
    if (_state.wifiSettings.empty())
    {
        return;
    }

    const size_t networkCount = _state.wifiSettings.size();

    // Skip exhausted networks before attempting
    while (_currentNetworkIndex < networkCount && _connectionAttempts >= WIFI_MAX_ATTEMPTS_PER_NETWORK)
    {
        _connectionAttempts = 0;
        _currentNetworkIndex++;
    }

    // If all networks exhausted (WIFI_MAX_ATTEMPTS_PER_NETWORK attempts per network), switch to AP mode
    if (_currentNetworkIndex >= networkCount)
    {
        ESP_LOGW(SVK_TAG, "All STA networks failed after %u attempts each. Switching to AP mode.", WIFI_MAX_ATTEMPTS_PER_NETWORK);
        switchToAPMode();
        return;
    }

    wifi_settings_t &network = _state.wifiSettings[_currentNetworkIndex];

    // Increment attempt counter
    _connectionAttempts++;

    ESP_LOGI(SVK_TAG, "Connecting to network: %s (attempt %u/%u)", network.ssid.c_str(), _connectionAttempts, WIFI_MAX_ATTEMPTS_PER_NETWORK);

    // Keep device awake while we retry WiFi (if PowerManager is available)
#if WIFI_HAS_POWER_MANAGER
    POWER::PowerManager::notifyActivity();
#endif
    configureNetwork(network);

    // Mark the start of a connection attempt to avoid overlapping begins
    _connectingSince = millis();

    // After max attempts on current network, mark for next network on subsequent call
    if (_connectionAttempts >= WIFI_MAX_ATTEMPTS_PER_NETWORK)
    {
        _connectionAttempts = WIFI_MAX_ATTEMPTS_PER_NETWORK; // clamp so while-loop above handles advance next tick
    }
}

void WiFiSettingsService::configureNetwork(wifi_settings_t &network)
{
    if (network.staticIPConfig)
    {
        // configure for static IP
        WiFi.config(network.localIP, network.gatewayIP, network.subnetMask, network.dnsIP1, network.dnsIP2);
    }
    else
    {
        // configure for DHCP
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    }
    WiFi.setHostname(_state.hostname.c_str());

    // attempt to connect to the network
    // IMPORTANT: do not force channel/BSSID here. For AP+STA, forcing these values
    // makes channel conflicts worse when AP is active.
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(network.ssid.c_str(), network.password.c_str());

#if CONFIG_IDF_TARGET_ESP32C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // https://www.wemos.cc/en/latest/c3/c3_mini_1_0_0.html#about-wifi
#endif
}



void WiFiSettingsService::switchToAPMode()
{
    ESP_LOGI(SVK_TAG, "Switching to AP-only mode (no auto-reconnect).");
    _apModeActive = true;
    _currentNetworkIndex = 0;
    _connectionAttempts = 0;
    _connectingSince = 0;

    // Stop mDNS before tearing down STA to avoid IGMP assertions when AP comes up
    MDNS.end();

    // Soft WiFi restart: disconnect STA
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);

    // Start AP via APSettingsService
    if (_apSettingsService)
    {
        _apSettingsService->forceAPMode();
    }
}
