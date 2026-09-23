/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <core/ESP32SvelteKit.h>

ESP32SvelteKit::ESP32SvelteKit(PsychicHttpServer *server, unsigned int numberEndpoints) : _server(server),
                                                                                          _numberEndpoints(numberEndpoints),
                                                                                          _featureService(server, &_socket),
                                                                                          _securitySettingsService(server, &ESPFS),
                                                                                          _wifiSettingsService(server, &ESPFS, &_securitySettingsService, &_socket),
                                                                                          _wifiScanner(server, &_securitySettingsService),
                                                                                          _wifiStatus(server, &_securitySettingsService, &_socket),
                                                                                          _apSettingsService(server, &ESPFS, &_securitySettingsService),
                                                                                          _apStatus(server, &_securitySettingsService, &_apSettingsService),

                                                                                          _socket(server, &_securitySettingsService, AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                          _notificationService(&_socket),
#if FT_ENABLED(FT_NTP)
                                                                                          _ntpSettingsService(server, &ESPFS, &_securitySettingsService),
                                                                                          _ntpStatus(server, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_SECURITY)
                                                                                          _authenticationService(server, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_SLEEP)
                                                                                          _sleepService(server, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_BATTERY)
                                                                                          _batteryService(&_socket),
#endif
                                                                                          _restartService(server, &_securitySettingsService),
                                                                                          _factoryResetService(server, &ESPFS, &_securitySettingsService),
                                                                                          _systemStatus(server, &_securitySettingsService)
{
}

void ESP32SvelteKit::begin()
{
    ESP_LOGV(SVK_TAG, "Loading settings from files system");
    ESPFS.begin(true);

    // IMPORTANT: initWiFi() may immediately switch to AP mode (manageSTA())
    // when there are no configured STA networks. Ensure the AP settings
    // service is wired before that happens, otherwise SoftAP won't start.
    _wifiSettingsService.setAPSettingsService(&_apSettingsService);
    _wifiSettingsService.initWiFi();

    // SvelteKit uses a lot of handlers, so we need to increase the max_uri_handlers
    // WWWData has 77 Endpoints, Framework has 27, and Lighstate Demo has 4
    _server->config.max_uri_handlers = _numberEndpoints;
    _server->listen(80);

#ifdef EMBED_WWW
    // Serve static resources from PROGMEM
    ESP_LOGV(SVK_TAG, "Registering routes from PROGMEM static resources");
    WWWData::registerRoutes(
        [&](const String &uri, const String &contentType, const uint8_t *content, size_t len)
        {
            PsychicHttpRequestCallback requestHandler = [contentType, content, len](PsychicRequest *request)
            {
                PsychicResponse response(request);
                response.setCode(200);
                response.setContentType(contentType.c_str());
                response.addHeader("Content-Encoding", "gzip");
                response.addHeader("Cache-Control", "public, immutable, max-age=31536000");
                response.setContent(content, len);
                return response.send();
            };
            PsychicWebHandler *handler = new PsychicWebHandler();
            handler->onRequest(requestHandler);
            _server->on(uri.c_str(), HTTP_GET, handler);

            // Set default end-point for all non matching requests
            // this is easier than using webServer.onNotFound()
            if (uri.equals("/index.html"))
            {
                _server->defaultEndpoint->setHandler(handler);
            }
        });
#else
    // Serve static resources from /www/
    ESP_LOGV(SVK_TAG, "Registering routes from FS /www/ static resources");
    _server->serveStatic("/_app/", ESPFS, "/www/_app/");
    _server->serveStatic("/favicon.png", ESPFS, "/www/favicon.png");
    //  Serving all other get requests with "/www/index.htm"
    _server->onNotFound([](PsychicRequest *request)
                        {
        if (request->method() == HTTP_GET) {
            PsychicFileResponse response(request, ESPFS, "/www/index.html", "text/html");
            return response.send();
            // String url = "http://" + request->host() + "/index.html";
            // request->redirect(url.c_str());
        } });
#endif

    // Serve static resources from /config/ if set by platformio.ini
#if SERVE_CONFIG_FILES
    _server->serveStatic("/config/", ESPFS, "/config/");
#endif

#if defined(ENABLE_CORS)
    ESP_LOGV(SVK_TAG, "Enabling CORS headers");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", CORS_ORIGIN);
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
#endif

    ESP_LOGV(SVK_TAG, "Starting MDNS");
    MDNS.begin(_wifiSettingsService.getHostname().c_str());
    MDNS.setInstanceName(_appName);
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 80);
    MDNS.addServiceTxt("http", "tcp", "Firmware Version", APP_VERSION);

#ifdef SERIAL_INFO
    ESP_LOGI(SVK_TAG, "Running Firmware Version: %s", APP_VERSION);
#endif

    // Start the services
    _apStatus.begin();
    _socket.begin();
    _notificationService.begin();
    _apSettingsService.begin();
    _factoryResetService.begin();
    _featureService.begin();
    _restartService.begin();
    _systemStatus.begin();
    _wifiSettingsService.begin();
    _wifiScanner.begin();
    _wifiStatus.begin();

#if FT_ENABLED(FT_NTP)
    _ntpSettingsService.begin();
    _ntpStatus.begin();
#endif

#if FT_ENABLED(FT_SECURITY)
    _authenticationService.begin();
    _securitySettingsService.begin();
#endif

#if FT_ENABLED(FT_SLEEP)
    _sleepService.begin();
    _sleepService.attachOnSleepCallback([&]()
                                        {   ESP_LOGI(SVK_TAG, "Attempting to stop server");
                                            for (auto client : _server->getClientList())
                                            {
                                                client->close();
                                            }
                                            vTaskDelete(_loopTaskHandle);
                                            ESP_LOGI(SVK_TAG, "Server stopped"); });
#endif

#if FT_ENABLED(FT_BATTERY)
    _batteryService.begin();
#endif

    // Start the loop task
    ESP_LOGV(SVK_TAG, "Starting loop task");
    xTaskCreatePinnedToCore(
        this->_loopImpl,            // Function that should be called
        "ESP32 SvelteKit Loop",     // Name of the task (for debugging)
        4096,                       // Stack size (bytes)
        this,                       // Pass reference to this class instance
        (tskIDLE_PRIORITY + 2),     // task priority
        &_loopTaskHandle,           // Task handle
        ESP32SVELTEKIT_RUNNING_CORE // Pin to application core
    );
}

void ESP32SvelteKit::_loop()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    bool wifi = false;
    bool ap = false;
    bool event = false;
    
    unsigned long lastRSSIEmit = 0;
    const unsigned long RSSI_EMIT_INTERVAL = 5000; // 5 seconds

    while (1)
    {
        _wifiSettingsService.loop(); // 30 seconds
        _apSettingsService.loop();   // 10 seconds

        // Query the connectivity status
        wifi = _wifiStatus.isConnected();
        ap = _apStatus.isActive();
        event = _socket.getConnectedClients() > 0;

        // Emit RSSI data periodically if clients are connected
        if (event && (millis() - lastRSSIEmit >= RSSI_EMIT_INTERVAL))
        {
            _wifiStatus.emitRSSI();
            lastRSSIEmit = millis();
        }

        // Update the system status
        if (wifi)
        {
            _connectionStatus = event ? ConnectionStatus::STA_CONNECTED : ConnectionStatus::STA;
        }
        else if (ap)
        {
            _connectionStatus = event ? ConnectionStatus::AP_CONNECTED : ConnectionStatus::AP;
        }
        else
        {
            _connectionStatus = ConnectionStatus::OFFLINE;
        }

        // iterate over all loop functions
        for (auto &function : _loopFunctions)
        {
            function();
        }

#ifdef TELEPLOT_TASKS
        static int lastTime = 0;
        if (millis() - lastTime > 1000)
        {
            lastTime = millis();
            ESP_LOGI(SVK_TAG, ">ESP32SveltekitTask:%i:%i", millis(), uxTaskGetStackHighWaterMark(NULL));
        }
#endif
        vTaskDelayUntil(&xLastWakeTime, ESP32SVELTEKIT_LOOP_INTERVAL / portTICK_PERIOD_MS);
    }
}
