#include "SensorConfig.h"
#include "LoggingConfig.h"
#include "../power/PowerManager.h"
#include "../system/Logging.h"
#include <PsychicRequest.h>
#include <PsychicResponse.h>
#include <PsychicJson.h>
#include <ArduinoJson.h>

#include <strings.h>

esp_err_t handleGetConfig(PsychicRequest *request) {
    auto& cal = SensorConfig::get();

    PsychicJsonResponse response(request);
    JsonVariant& root = response.getRoot();
    root["tempOffset"] = cal.tempOffset;
    root["humidOffset"] = cal.humidOffset;
    root["luxOffset"] = cal.luxOffset;
    root["soilOffset"] = cal.soilOffset;
    root["soilMin"] = cal.soilMin;
    root["soilMax"] = cal.soilMax;
    root["batAdcMin"] = cal.batAdcMin;
    root["batAdcMax"] = cal.batAdcMax;

    auto logCfg = LoggingConfig::get();
    JsonObject logging = root["logging"].to<JsonObject>();
    logging["level"] = LOG::Logging::levelToString(logCfg.level);
    logging["ringBufferSize"] = logCfg.ringBufferSize;

    return response.send();
}

esp_err_t handleSaveConfig(PsychicRequest *request) {
    String body = request->body();
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
        return request->reply(400, "text/plain", "Invalid JSON");
    }
    
    auto& cal = SensorConfig::get();
    
    if (doc["tempOffset"].is<float>()) cal.tempOffset = doc["tempOffset"];
    if (doc["humidOffset"].is<float>()) cal.humidOffset = doc["humidOffset"];
    if (doc["luxOffset"].is<float>()) cal.luxOffset = doc["luxOffset"];
    if (doc["soilOffset"].is<float>()) cal.soilOffset = doc["soilOffset"];
    if (doc["soilMin"].is<int>()) cal.soilMin = doc["soilMin"];
    if (doc["soilMax"].is<int>()) cal.soilMax = doc["soilMax"];
    if (doc["batAdcMin"].is<int>()) cal.batAdcMin = doc["batAdcMin"];
    if (doc["batAdcMax"].is<int>()) cal.batAdcMax = doc["batAdcMax"];

    // Logging config
    if (doc["logging"].is<JsonObject>()) {
        auto logging = doc["logging"].as<JsonObject>();

        // API: logging.level (strict)
        if (logging["level"].is<const char *>()) {
            const char* levelStr = logging["level"].as<const char*>();
            if (!levelStr) {
                return request->reply(400, "text/plain", "Invalid logging.level");
            }

            // Use existing stringToLevel() to avoid code duplication
            esp_log_level_t lvl = LOG::Logging::stringToLevel(
                String(levelStr), 
                ESP_LOG_NONE  // fallback value
            );
            
            // Validate: reject invalid strings (when fallback was used for non-"none")
            if (lvl == ESP_LOG_NONE && strcasecmp(levelStr, "none") != 0) {
                return request->reply(400, "text/plain", "Invalid logging.level");
            }

            LoggingConfig::setLevel(lvl);
        }

        // Ring buffer size is now hard-coded to 20 (no runtime config)

        auto applied = LoggingConfig::get();
        LOG::Logging::setSettings(applied);
        LOGI("[Logging] Updated: level=%s ring=%u", LOG::Logging::levelToString(applied.level), applied.ringBufferSize);
    }
    
    SensorConfig::save();
    
    return request->reply(200, "text/plain", "Config saved");
}
