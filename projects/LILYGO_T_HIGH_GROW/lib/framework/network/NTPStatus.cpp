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

#include <network/NTPStatus.h>

NTPStatus::NTPStatus(PsychicHttpServer *server, SecurityManager *securityManager) : _server(server),
                                                                                    _securityManager(securityManager)
{
}

void NTPStatus::begin()
{
    _server->on(NTP_STATUS_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&NTPStatus::ntpStatus, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));

    ESP_LOGV(SVK_TAG, "Registered GET endpoint: %s", NTP_STATUS_SERVICE_PATH);
}

/*
 * Formats the time using the format provided.
 *
 * Uses a 25 byte buffer, large enough to fit an ISO time string with offset.
 */
String formatTime(tm *time, const char *format)
{
    char time_string[25];
    strftime(time_string, 25, format, time);
    return String(time_string);
}

// Formats ISO8601 with numeric offset and colon (e.g. 2025-12-15T10:03:07+01:00)
String toLocalTimeString(tm *time)
{
    char date_part[25];
    strftime(date_part, sizeof(date_part), "%FT%T", time);

    char offset[6]; // +hhmm
    strftime(offset, sizeof(offset), "%z", time);

    if (strlen(offset) == 5)
    {
        char with_offset[32];
        // Insert colon before last two digits
        // offset: +hhmm -> +hh:mm
        snprintf(with_offset, sizeof(with_offset), "%s%c%c%c:%c%c", date_part, offset[0], offset[1], offset[2], offset[3], offset[4]);
        return String(with_offset);
    }

    // Fallback without offset
    return String(date_part);
}

String toUTCTimeString(tm *time)
{
    return formatTime(time, "%FT%TZ");
}

esp_err_t NTPStatus::ntpStatus(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject root = response.getRoot();

    // grab the current instant in unix seconds
    time_t now = time(nullptr);

    // only provide enabled/disabled status for now
    root["status"] = sntp_enabled() ? 1 : 0;

    // the current time in UTC
    root["utc_time"] = toUTCTimeString(gmtime(&now));

    // local time with offset
    root["local_time"] = toLocalTimeString(localtime(&now));

    // the sntp server name (guard against null)
    const char* serverName = sntp_getservername(0);
    root["server"] = serverName ? serverName : "";

    // device uptime in seconds
    root["uptime"] = millis() / 1000;

    return response.send();
}
