/**
 * @file Rf433ApiService.cpp
 * @brief RF433 HTTP API implementation
 */

#include "Rf433ApiService.h"
#include "../../power/PowerManager.h"
#include "../../system/Logging.h"
#include "../../hardware/rf433/core/Rf433RadioController.h"
#include <ArduinoJson.h>
#include <PsychicJson.h>

#undef LOG_TAG
#define LOG_TAG "ApiRF433"

namespace API {

Rf433ApiService::Rf433ApiService(PsychicHttpServer* server,
                                 SecurityManager* securityManager,
                                 Rf433Manager* manager,
                                 Rf433SettingsService* settingsService)
    : _server(server),
      _securityManager(securityManager),
      _manager(manager),
      _settingsService(settingsService) {}

void Rf433ApiService::begin() {
    // Admin-only wrapper with power activity tracking
    auto adminWrap = [this](auto fn) {
        return _securityManager->wrapRequest(
            [fn](PsychicRequest* request) -> esp_err_t {
                POWER::PowerManager::notifyActivity("api/rf433");
                return fn(request);
            },
            AuthenticationPredicates::IS_ADMIN);
    };
    
    // Register endpoints
    _server->on("/api/rf433/devices", HTTP_GET, adminWrap([this](PsychicRequest* req) {
        return handleGetDevices(req);
    }));
    
    _server->on("/api/rf433/devices", HTTP_POST, adminWrap([this](PsychicRequest* req) {
        return handleAddDevice(req);
    }));
    
    _server->on("/api/rf433/devices/*", HTTP_PUT, adminWrap([this](PsychicRequest* req) {
        return handleUpdateDevice(req);
    }));
    
    _server->on("/api/rf433/devices/*", HTTP_DELETE, adminWrap([this](PsychicRequest* req) {
        return handleDeleteDevice(req);
    }));
    
    _server->on("/api/rf433/send", HTTP_POST, adminWrap([this](PsychicRequest* req) {
        return handleSendCommand(req);
    }));
    
    _server->on("/api/rf433/state", HTTP_GET, adminWrap([this](PsychicRequest* req) {
        return handleGetState(req);
    }));
    
    LOGI("RF433 API endpoints registered");
}

// ============================================================================
// GET /api/rf433/devices - List all devices
// ============================================================================

esp_err_t Rf433ApiService::handleGetDevices(PsychicRequest* request) {
    const auto& devices = _manager->getDevices();
    
    PsychicJsonResponse response(request);
    JsonVariant& root = response.getRoot();
    
    JsonArray devicesArray = root["devices"].to<JsonArray>();
    for (const auto& device : devices) {
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
    
    root["count"] = devices.size();
    root["max_devices"] = RF433::MAX_DEVICES;
    
    return response.send();
}

// ============================================================================
// POST /api/rf433/devices - Add new device
// ============================================================================

esp_err_t Rf433ApiService::handleAddDevice(PsychicRequest* request) {
    PsychicJsonResponse response(request);
    
    // Parse request body
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, request->body().c_str());
    if (error) {
        LOGW("JSON parse error: %s", error.c_str());
        response.setCode(400);
        response.getRoot()["error"] = "Invalid JSON";
        return response.send();
    }
    
    // Build device from JSON
    Rf433Device device;
    device.id = doc["id"] | "";
    device.label = doc["label"] | "";
    device.codeOn = doc["code_on"] | 0;
    device.codeOff = doc["code_off"] | 0;
    device.bitLength = doc["bit_length"] | RF433::DEFAULT_BIT_LENGTH;
    device.protocol = doc["protocol"] | RF433::DEFAULT_PROTOCOL;
    device.pulseLength = doc["pulse_length"] | RF433::DEFAULT_PULSE_LENGTH;
    device.repeat = doc["repeat"] | RF433::DEFAULT_REPEAT_COUNT;
    
    // Validate and add
    if (!device.isValid()) {
        LOGW("Device validation failed: id=%s", device.id.c_str());
        response.setCode(400);
        response.getRoot()["error"] = "Invalid device data";
        return response.send();
    }
    
    if (!_manager->addDevice(device)) {
        LOGW("Failed to add device: id=%s", device.id.c_str());
        response.setCode(409);
        response.getRoot()["error"] = "Device already exists or limit reached";
        return response.send();
    }
    
    // Persist to filesystem
    _settingsService->save();
    
    LOGI("Device added: id=%s label='%s'", device.id.c_str(), device.label.c_str());
    
    response.getRoot()["success"] = true;
    response.getRoot()["id"] = device.id;
    return response.send();
}

// ============================================================================
// PUT /api/rf433/devices/:id - Update device
// ============================================================================

esp_err_t Rf433ApiService::handleUpdateDevice(PsychicRequest* request) {
    PsychicJsonResponse response(request);
    
    // Extract device ID from path (/api/rf433/devices/:id)
    String path = request->path();
    int lastSlash = path.lastIndexOf('/');
    if (lastSlash < 0) {
        response.setCode(400);
        response.getRoot()["error"] = "Invalid path";
        return response.send();
    }
    String deviceId = path.substring(lastSlash + 1);
    
    // Parse request body
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, request->body().c_str());
    if (error) {
        LOGW("JSON parse error: %s", error.c_str());
        response.setCode(400);
        response.getRoot()["error"] = "Invalid JSON";
        return response.send();
    }
    
    // Build updated device
    Rf433Device device;
    device.id = doc["id"] | deviceId;
    device.label = doc["label"] | "";
    device.codeOn = doc["code_on"] | 0;
    device.codeOff = doc["code_off"] | 0;
    device.bitLength = doc["bit_length"] | RF433::DEFAULT_BIT_LENGTH;
    device.protocol = doc["protocol"] | RF433::DEFAULT_PROTOCOL;
    device.pulseLength = doc["pulse_length"] | RF433::DEFAULT_PULSE_LENGTH;
    device.repeat = doc["repeat"] | RF433::DEFAULT_REPEAT_COUNT;
    
    // Validate
    if (!device.isValid()) {
        LOGW("Device validation failed: id=%s", device.id.c_str());
        response.setCode(400);
        response.getRoot()["error"] = "Invalid device data";
        return response.send();
    }
    
    // Update
    if (!_manager->updateDevice(deviceId, device)) {
        LOGW("Failed to update device: id=%s", deviceId.c_str());
        response.setCode(404);
        response.getRoot()["error"] = "Device not found";
        return response.send();
    }
    
    // Persist to filesystem
    _settingsService->save();
    
    LOGI("Device updated: id=%s label='%s'", device.id.c_str(), device.label.c_str());
    
    response.getRoot()["success"] = true;
    return response.send();
}

// ============================================================================
// DELETE /api/rf433/devices/:id - Remove device
// ============================================================================

esp_err_t Rf433ApiService::handleDeleteDevice(PsychicRequest* request) {
    PsychicJsonResponse response(request);
    
    // Extract device ID from path
    String path = request->path();
    int lastSlash = path.lastIndexOf('/');
    if (lastSlash < 0) {
        response.setCode(400);
        response.getRoot()["error"] = "Invalid path";
        return response.send();
    }
    String deviceId = path.substring(lastSlash + 1);
    
    // Remove
    if (!_manager->removeDevice(deviceId)) {
        LOGW("Failed to remove device: id=%s", deviceId.c_str());
        response.setCode(404);
        response.getRoot()["error"] = "Device not found";
        return response.send();
    }
    
    // Persist to filesystem
    _settingsService->save();
    
    LOGI("Device removed: id=%s", deviceId.c_str());
    
    response.getRoot()["success"] = true;
    return response.send();
}

// ============================================================================
// POST /api/rf433/send - Send command
// ============================================================================

esp_err_t Rf433ApiService::handleSendCommand(PsychicRequest* request) {
    PsychicJsonResponse response(request);
    
    // Check if RF433 is enabled
    if (!_settingsService->isEnabled()) {
        LOGW("RF433 is disabled");
        response.setCode(503);
        response.getRoot()["error"] = "RF433 is disabled";
        return response.send();
    }
    
    // Check if RadioController is ready
    if (!Rf433RadioController::isReady()) {
        LOGW("RadioController not ready");
        response.setCode(503);
        response.getRoot()["error"] = "RadioController not initialized";
        return response.send();
    }
    
    // Parse request body
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, request->body().c_str());
    if (error) {
        LOGW("JSON parse error: %s", error.c_str());
        response.setCode(400);
        response.getRoot()["error"] = "Invalid JSON";
        return response.send();
    }
    
    // Extract parameters
    String deviceId = doc["device_id"] | "";
    String command = doc["command"] | "";
    bool sync = doc["sync"] | false;
    
    // Validate
    if (deviceId.isEmpty()) {
        response.setCode(400);
        response.getRoot()["error"] = "Missing device_id";
        return response.send();
    }
    
    if (command != "on" && command != "off") {
        response.setCode(400);
        response.getRoot()["error"] = "Invalid command (use 'on' or 'off')";
        return response.send();
    }
    
    // Send command
    bool commandOn = (command == "on");
    bool success = _manager->sendCommand(deviceId, commandOn, !sync);
    
    if (!success) {
        LOGW("Send command failed: device=%s cmd=%s", deviceId.c_str(), command.c_str());
        response.setCode(500);
        response.getRoot()["error"] = "Transmission failed";
        return response.send();
    }
    
    response.getRoot()["success"] = true;
    response.getRoot()["device_id"] = deviceId;
    response.getRoot()["command"] = command;
    return response.send();
}

// ============================================================================
// GET /api/rf433/state - Get device states
// ============================================================================

esp_err_t Rf433ApiService::handleGetState(PsychicRequest* request) {
    const auto& states = _manager->getDeviceStates();
    
    PsychicJsonResponse response(request);
    JsonVariant& root = response.getRoot();
    
    JsonArray statesArray = root["states"].to<JsonArray>();
    for (const auto& state : states) {
        JsonObject stateObj = statesArray.add<JsonObject>();
        stateObj["device_id"] = state.deviceId;
        stateObj["last_command_on"] = state.lastCommandOn;
        stateObj["last_transmit_ms"] = state.lastTransmitMs;
        
        uint32_t age_ms = millis() - state.lastTransmitMs;
        stateObj["age_ms"] = age_ms;
        stateObj["age_sec"] = age_ms / 1000;
    }
    
    root["count"] = states.size();
    root["controller_ready"] = Rf433RadioController::isReady();
    root["rf433_enabled"] = _settingsService->isEnabled();
    
    return response.send();
}

}  // namespace API
