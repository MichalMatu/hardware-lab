/**
 * @file Rf433SettingsService.h
 * @brief Persistent settings service for RF433 devices
 * 
 * Uses ESP32-SvelteKit framework for:
 * - LittleFS persistence (JSON file)
 * - HTTP endpoint for settings management
 * - State change notifications
 */

#pragma once

#include <core/StatefulService.h>
#include <core/HttpEndpoint.h>
#include <core/FSPersistence.h>
#include "../../../config/Rf433Config.h"
#include "../devices/Rf433Manager.h"

#define RF433_SETTINGS_FILE "/config/rf433Settings.json"
#define RF433_SETTINGS_SERVICE_PATH "/rest/rf433Settings"

/**
 * @brief Settings container for RF433 configuration
 * 
 * Includes global settings and device list.
 * Serialized to/from JSON via StatefulService pattern.
 */
class Rf433SettingsData {
public:
    Rf433Settings settings;
    std::vector<Rf433Device> devices;
    
    /**
     * @brief Serialize settings to JSON
     */
    static void read(Rf433SettingsData& data, JsonObject& root);
    
    /**
     * @brief Deserialize settings from JSON
     */
    static StateUpdateResult update(JsonObject& root, Rf433SettingsData& data, const String& originId);
};

/**
 * @brief RF433 settings service with persistence
 * 
 * Manages RF433 configuration persistence and provides HTTP endpoint.
 * Integrates with Rf433Manager for device sync.
 */
class Rf433SettingsService : public StatefulService<Rf433SettingsData> {
public:
    Rf433SettingsService(PsychicHttpServer* server, 
                         FS* fs, 
                         SecurityManager* securityManager,
                         Rf433Manager* manager);
    
    /**
     * @brief Initialize service (load from FS, register HTTP endpoint)
     */
    void begin();
    
    /**
     * @brief Get current settings
     */
    const Rf433Settings& getSettings() const { return _state.settings; }
    
    /**
     * @brief Get all devices
     */
    const std::vector<Rf433Device>& getDevices() const { return _state.devices; }
    
    /**
     * @brief Check if RF433 is enabled
     */
    bool isEnabled() const { return _state.settings.enabled; }
    
    /**
     * @brief Manually trigger save to filesystem
     */
    void save();
    
private:
    HttpEndpoint<Rf433SettingsData> _httpEndpoint;
    FSPersistence<Rf433SettingsData> _fsPersistence;
    Rf433Manager* _manager;
    
    /**
     * @brief Called when settings are updated via HTTP or FS load
     */
    void onConfigUpdated();
    
    /**
     * @brief Sync devices from settings to manager
     */
    void syncDevicesToManager();
};
