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

#include <network/NTPSettingsService.h>

NTPSettingsService::NTPSettingsService(PsychicHttpServer *server,
                                       FS *fs,
                                       SecurityManager *securityManager) : _server(server),
                                                                           _securityManager(securityManager),
                                                                           _httpEndpoint(NTPSettings::read, NTPSettings::update, this, server, NTP_SETTINGS_SERVICE_PATH, securityManager),
                                                                           _fsPersistence(NTPSettings::read, NTPSettings::update, this, fs, NTP_SETTINGS_FILE)
{
    addUpdateHandler([&](const String &originId)
                     { configureNTP(); },
                     false);
}

void NTPSettingsService::begin()
{
    WiFi.onEvent(
        std::bind(&NTPSettingsService::onNetworkDisconnected, this, std::placeholders::_1, std::placeholders::_2),
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(std::bind(&NTPSettingsService::onNetworkGotIP, this, std::placeholders::_1, std::placeholders::_2),
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

    _httpEndpoint.begin();
    _server->on(TIME_PATH,
                HTTP_POST,
                _securityManager->wrapCallback(
                    std::bind(&NTPSettingsService::configureTime, this, std::placeholders::_1, std::placeholders::_2),
                    AuthenticationPredicates::IS_ADMIN));

    ESP_LOGV(SVK_TAG, "Registered POST endpoint: %s", TIME_PATH);

    _fsPersistence.readFromFS();
    configureNTP();
}

void NTPSettingsService::onNetworkGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    ESP_LOGI(SVK_TAG, "Got IP address, starting NTP synchronization");
    configureNTP();
}

void NTPSettingsService::onNetworkDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    ESP_LOGI(SVK_TAG, "Network connection dropped, stopping NTP");
    configureNTP();
}

void NTPSettingsService::configureNTP()
{
    // Ensure we always have a valid TZ string; fallback to UTC0 if not set
    String tzString = _state.tzFormat.length() ? _state.tzFormat : String("UTC0");

    // Guard against mismatched tz_label/tz_format from older configs.
    // Europe/Warsaw is CET/CEST (UTC+1 winter, UTC+2 summer). If tz_format is EET/EEST
    // (UTC+2/UTC+3) then local time will be off by +1h in winter.
    if (_state.tzLabel == "Europe/Warsaw")
    {
        if (tzString == "EET-2EEST,M3.5.0/3,M10.5.0/4" || tzString.startsWith("EET-2EEST"))
        {
            ESP_LOGW(SVK_TAG, "Correcting invalid tz_format for %s: %s -> CET-1CEST,M3.5.0,M10.5.0/3",
                     _state.tzLabel.c_str(), tzString.c_str());
            tzString = "CET-1CEST,M3.5.0,M10.5.0/3";
        }
    }

    bool networkConnected = WiFi.isConnected();

    // Apply TZ immediately so localtime()/gmtime() use the expected offset even before SNTP completes
    setenv("TZ", tzString.c_str(), 1);
    tzset();

    if (networkConnected && _state.enabled)
    {
        ESP_LOGI(SVK_TAG, "Starting NTP...");
        configTzTime(tzString.c_str(), _state.server.c_str());
    }
    else
    {

#ifdef CONFIG_LWIP_TCPIP_CORE_LOCKING
        if (!sys_thread_tcpip(LWIP_CORE_LOCK_QUERY_HOLDER))
        {
            LOCK_TCPIP_CORE();
        }
#endif
        sntp_stop();

#ifdef CONFIG_LWIP_TCPIP_CORE_LOCKING
        if (sys_thread_tcpip(LWIP_CORE_LOCK_QUERY_HOLDER))
        {
            UNLOCK_TCPIP_CORE();
        }
#endif
    }
}

esp_err_t NTPSettingsService::configureTime(PsychicRequest *request, JsonVariant &json)
{
    // Ensure NTP is disabled before allowing manual time set
    if (sntp_enabled()) {
        ESP_LOGW(SVK_TAG, "Manual time rejected: NTP is enabled. Disable NTP first.");
        return request->reply(400, "application/json", 
            "{\"status\":\"error\",\"message\":\"NTP must be disabled before setting manual time\"}");
    }
    
    if (!json.is<JsonObject>()) {
        return request->reply(400, "application/json", 
            "{\"status\":\"error\",\"message\":\"Invalid JSON payload\"}");
    }
    
    struct tm tm = {0};
    String timeLocal = json["local_time"];
    
    if (timeLocal.isEmpty()) {
        return request->reply(400, "application/json", 
            "{\"status\":\"error\",\"message\":\"Missing local_time field\"}");
    }
    
    char *s = strptime(timeLocal.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
    if (s == nullptr) {
        ESP_LOGW(SVK_TAG, "Manual time rejected: invalid format '%s'", timeLocal.c_str());
        return request->reply(400, "application/json", 
            "{\"status\":\"error\",\"message\":\"Invalid time format. Expected: YYYY-MM-DDTHH:MM:SS\"}");
    }
    
    // Validate year range (2025-2040)
    int year = tm.tm_year + 1900;
    if (year < 2025 || year > 2040) {
        ESP_LOGW(SVK_TAG, "Manual time rejected: year %d out of range [2025-2040]", year);
        char errorMsg[128];
        snprintf(errorMsg, sizeof(errorMsg), 
            "{\"status\":\"error\",\"message\":\"Year %d out of valid range (2025-2040)\"}", year);
        return request->reply(400, "application/json", errorMsg);
    }
    
    time_t time = mktime(&tm);
    struct timeval now = {.tv_sec = time};

        // Debug: log old/new system time (local) before and after settimeofday
        time_t before = ::time(nullptr);
        struct tm beforeLocal;
        localtime_r(&before, &beforeLocal);

    settimeofday(&now, nullptr);

        time_t after = ::time(nullptr);
        struct tm afterLocal;
        localtime_r(&after, &afterLocal);

        ESP_LOGI(SVK_TAG,
             "Manual time set: old=%04d-%02d-%02dT%02d:%02d:%02d new=%04d-%02d-%02dT%02d:%02d:%02d tz=%s",
             beforeLocal.tm_year + 1900, beforeLocal.tm_mon + 1, beforeLocal.tm_mday,
             beforeLocal.tm_hour, beforeLocal.tm_min, beforeLocal.tm_sec,
             afterLocal.tm_year + 1900, afterLocal.tm_mon + 1, afterLocal.tm_mday,
             afterLocal.tm_hour, afterLocal.tm_min, afterLocal.tm_sec,
             getenv("TZ") ? getenv("TZ") : "");
    
    return request->reply(200, "application/json", 
        "{\"status\":\"success\",\"message\":\"Time set successfully\"}");
}
