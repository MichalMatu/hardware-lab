/**
 * @file Rf433ApiService.h
 * @brief HTTP API for RF433 device management and transmission
 * 
 * Endpoints:
 * - GET    /api/rf433/devices       - List all devices
 * - POST   /api/rf433/devices       - Add new device
 * - PUT    /api/rf433/devices/:id   - Update device
 * - DELETE /api/rf433/devices/:id   - Remove device
 * - POST   /api/rf433/send          - Send command
 * - GET    /api/rf433/state         - Get device states
 */

#pragma once

#include <PsychicHttpServer.h>
#include <security/SecurityManager.h>
#include "../../hardware/rf433/devices/Rf433Manager.h"
#include "../../hardware/rf433/settings/Rf433SettingsService.h"

namespace API {

class Rf433ApiService {
public:
    Rf433ApiService(PsychicHttpServer* server,
                    SecurityManager* securityManager,
                    Rf433Manager* manager,
                    Rf433SettingsService* settingsService);
    
    void begin();

private:
    // GET /api/rf433/devices - List all devices
    esp_err_t handleGetDevices(PsychicRequest* request);
    
    // POST /api/rf433/devices - Add new device
    esp_err_t handleAddDevice(PsychicRequest* request);
    
    // PUT /api/rf433/devices/:id - Update device
    esp_err_t handleUpdateDevice(PsychicRequest* request);
    
    // DELETE /api/rf433/devices/:id - Remove device
    esp_err_t handleDeleteDevice(PsychicRequest* request);
    
    // POST /api/rf433/send - Send command
    esp_err_t handleSendCommand(PsychicRequest* request);
    
    // GET /api/rf433/state - Get device states
    esp_err_t handleGetState(PsychicRequest* request);
    
    PsychicHttpServer* _server;
    SecurityManager* _securityManager;
    Rf433Manager* _manager;
    Rf433SettingsService* _settingsService;
};

}  // namespace API
