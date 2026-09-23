/**
 * @file Rf433SettingsService.cpp
 * @brief RF433 settings persistence implementation
 */

#include "Rf433SettingsService.h"
#include "../../../system/Logging.h"
#include <ArduinoJson.h>

#undef LOG_TAG
#define LOG_TAG "RF433Settings"

// ============================================================================
// Rf433SettingsData serialization
// ============================================================================

void Rf433SettingsData::read(Rf433SettingsData& data, JsonObject& root) {
    // Global settings
    JsonObject settingsObj = root["settings"].to<JsonObject>();
    settingsObj["enabled"] = data.settings.enabled;
    settingsObj["power_pin"] = data.settings.powerPin;
    settingsObj["tx_pin"] = data.settings.txPin;
    
    // Device list
    JsonArray devicesArray = root["devices"].to<JsonArray>();
    for (const auto& device : data.devices) {
        JsonObject deviceObj = devicesArray.add<JsonObject>();
        deviceObj["id"] = device.id;
        deviceObj["label"] = device.label;
        deviceObj["code_on"] = device.codeOn;
        deviceObj["code_off"] = device.codeOff;
        deviceObj["bit_length"] = device.bitLength;
        deviceObj["protocol"] = device.protocol;
        deviceObj["pulse_length"] = device.pulseLength;
        deviceObj["repeat"] = device.repeat;
    }
}

StateUpdateResult Rf433SettingsData::update(JsonObject& root, Rf433SettingsData& data, const String& originId) {
    bool changed = false;
    
    // Global settings
    if (root["settings"].is<JsonObject>()) {
        JsonObject settingsObj = root["settings"];
        
        bool newEnabled = settingsObj["enabled"] | data.settings.enabled;
        if (newEnabled != data.settings.enabled) {
            data.settings.enabled = newEnabled;
            changed = true;
        }
        
        uint8_t newPowerPin = settingsObj["power_pin"] | data.settings.powerPin;
        if (newPowerPin != data.settings.powerPin) {
            data.settings.powerPin = newPowerPin;
            changed = true;
        }
        
        uint8_t newTxPin = settingsObj["tx_pin"] | data.settings.txPin;
        if (newTxPin != data.settings.txPin) {
            data.settings.txPin = newTxPin;
            changed = true;
        }
    }
    
    // Device list
    if (root["devices"].is<JsonArray>()) {
        JsonArray devicesArray = root["devices"];
        data.devices.clear();
        data.devices.reserve(devicesArray.size());
        
        for (JsonObject deviceObj : devicesArray) {
            Rf433Device device;
            device.id = deviceObj["id"] | "";
            device.label = deviceObj["label"] | "";
            device.codeOn = deviceObj["code_on"] | 0;
            device.codeOff = deviceObj["code_off"] | 0;
            device.bitLength = deviceObj["bit_length"] | RF433::DEFAULT_BIT_LENGTH;
            device.protocol = deviceObj["protocol"] | RF433::DEFAULT_PROTOCOL;
            device.pulseLength = deviceObj["pulse_length"] | RF433::DEFAULT_PULSE_LENGTH;
            device.repeat = deviceObj["repeat"] | RF433::DEFAULT_REPEAT_COUNT;
            
            // Only add valid devices
            if (device.isValid()) {
                data.devices.push_back(device);
            } else {
                LOGW("Skipping invalid device during load: id=%s", device.id.c_str());
            }
        }
        changed = true;
    }
    
    return changed ? StateUpdateResult::CHANGED : StateUpdateResult::UNCHANGED;
}

// ============================================================================
// Rf433SettingsService implementation
// ============================================================================

Rf433SettingsService::Rf433SettingsService(PsychicHttpServer* server,
                                           FS* fs,
                                           SecurityManager* securityManager,
                                           Rf433Manager* manager)
    : _httpEndpoint(Rf433SettingsData::read,
                    Rf433SettingsData::update,
                    this,
                    server,
                    RF433_SETTINGS_SERVICE_PATH,
                    securityManager,
                    AuthenticationPredicates::IS_ADMIN),
      _fsPersistence(Rf433SettingsData::read,
                     Rf433SettingsData::update,
                     this,
                     fs,
                     RF433_SETTINGS_FILE),
      _manager(manager) {
    
    // Register update handler
    addUpdateHandler([&](const String& originId) { onConfigUpdated(); }, false);
}

void Rf433SettingsService::begin() {
    // Register HTTP endpoint
    _httpEndpoint.begin();
    
    // Load from filesystem
    _fsPersistence.readFromFS();
    
    // Sync devices to manager
    syncDevicesToManager();
    
    LOGI("Settings loaded: enabled=%d devices=%u",
         _state.settings.enabled ? 1 : 0,
         static_cast<unsigned>(_state.devices.size()));
}

void Rf433SettingsService::save() {
    // Update state from manager (sync back)
    _state.devices.clear();
    for (const auto& device : _manager->getDevices()) {
        _state.devices.push_back(device);
    }
    
    // Persist to filesystem
    _fsPersistence.writeToFS();
    
    LOGD("Settings saved: devices=%u", static_cast<unsigned>(_state.devices.size()));
}

void Rf433SettingsService::onConfigUpdated() {
    LOGI("Settings updated: enabled=%d devices=%u",
         _state.settings.enabled ? 1 : 0,
         static_cast<unsigned>(_state.devices.size()));
    
    // Sync devices to manager
    syncDevicesToManager();
    
    // Auto-save after update
    _fsPersistence.writeToFS();
}

void Rf433SettingsService::syncDevicesToManager() {
    if (_manager == nullptr) {
        LOGW("Manager not set, cannot sync devices");
        return;
    }
    
    // Clear existing devices in manager
    _manager->clearDevices();
    
    // Add all devices from settings
    for (const auto& device : _state.devices) {
        if (!_manager->addDevice(device)) {
            LOGW("Failed to add device to manager: id=%s", device.id.c_str());
        }
    }
    
    LOGD("Synced %u devices to manager", static_cast<unsigned>(_state.devices.size()));
}
