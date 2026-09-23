# RF433 Module

## Overview

The RF433 module enables control of 433MHz RF devices (e.g., power outlets, lamps, remote switches) through the ESP32-SvelteKit web interface. The module uses the `rc-switch` library and supports 12 different RF protocols.

## Hardware Configuration

- **Power Pin**: GPIO 23 (MOSFET control for power gating)
- **TX Data Pin**: GPIO 17 (433MHz transmitter data)
- **Library**: rc-switch 2.6.4 (12 protocols supported)

## Features

✅ **Device Management**
- Add/edit/delete RF433 devices via web UI
- Store up to 32 devices in LittleFS
- Each device has separate ON/OFF codes (hex format)
- Configurable protocol (1-12), bit length (1-32), pulse length, and repeats

✅ **Asynchronous Transmission**
- FreeRTOS task-based TX (runs on CPU1)
- Non-blocking API calls
- Queue-based command dispatch (8 slots)
- Power-gated (GPIO 23) for battery optimization

✅ **State Tracking**
- Tracks last command (ON/OFF) per device
- Displays device status in web UI
- Command history with timestamps

✅ **Security**
- All endpoints protected with JWT authentication
- Admin-only access (IS_ADMIN predicate)

## API Endpoints

All endpoints require authentication (`Authorization: Bearer <token>`).

### Device Management

#### GET `/api/rf433/devices`
List all configured RF433 devices.

**Response:**
```json
{
  "devices": [
    {
      "id": "lamp_living_room",
      "label": "Living Room Lamp",
      "code_on": 5393,
      "code_off": 5396,
      "bit_length": 24,
      "protocol": 1,
      "pulse_length": 0,
      "repeat": 4
    }
  ]
}
```

#### POST `/api/rf433/devices`
Add a new device.

**Request:**
```json
{
  "id": "lamp_bedroom",
  "label": "Bedroom Lamp",
  "code_on": 5505,
  "code_off": 5508,
  "bit_length": 24,
  "protocol": 1,
  "pulse_length": 0,
  "repeat": 4
}
```

#### PUT `/api/rf433/devices/:id`
Update an existing device (ID cannot be changed).

#### DELETE `/api/rf433/devices/:id`
Remove a device.

### Command Execution

#### POST `/api/rf433/send`
Send a command to a device.

**Request:**
```json
{
  "device_id": "lamp_living_room",
  "command": "on",
  "sync": false
}
```

**Response:**
```json
{
  "success": true,
  "device_id": "lamp_living_room",
  "command": "on"
}
```

#### GET `/api/rf433/state`
Get controller status and device states.

**Response:**
```json
{
  "controller_ready": true,
  "states": [
    {
      "device_id": "lamp_living_room",
      "last_command_on": true,
      "last_transmit_ms": 123456789,
      "age_ms": 5000,
      "age_sec": 5
    }
  ]
}
```

## Web UI

The web interface is located at `/settings/integrations/433` and provides:

- **Device List**: Table view with ON/OFF codes, protocol, and status
- **Control Buttons**: Quick ON/OFF commands for each device
- **Add/Edit Modal**: Full CRUD form with validation
- **Status Indicator**: Shows controller ready/not ready state
- **Real-time Updates**: Device state refreshes after commands

### Form Validation

- **ID**: Required, alphanumeric + dash/underscore only, cannot be changed after creation
- **Label**: Required
- **ON/OFF Codes**: Required, hex format (e.g., `0x1234` or `1234`)
- **Bit Length**: 1-32 (default: 24)
- **Protocol**: 1-12 (default: 1)
- **Pulse Length**: 0+ microseconds (0 = auto)
- **Repeats**: 1-10 (default: 4)

## Architecture

### Backend Components

1. **Rf433Config.h** - Data structures and validation
2. **Rf433RadioController** - FreeRTOS task, async TX, power management
3. **Rf433Manager** - CRUD operations, state tracking
4. **Rf433SettingsService** - LittleFS persistence (StatefulService pattern)
5. **Rf433ApiService** - REST endpoints with security

### Frontend Components

1. **rf433.ts** - TypeScript type definitions
2. **DeviceFormModal.svelte** - Add/edit form with validation
3. **+page.svelte** - Main UI (device list, controls, CRUD)

### Memory Usage

```
Flash Code: +48 KB
DRAM:       +472 bytes
```

## Usage Example

1. Navigate to `/settings/integrations/433`
2. Click "Add Device"
3. Fill in device details:
   - ID: `outlet_fan`
   - Label: `Living Room Fan Outlet`
   - ON Code: `0x151400` (hex)
   - OFF Code: `0x151403` (hex)
   - Protocol: 1
   - Bit Length: 24
   - Repeats: 4
4. Click "Save"
5. Test with ON/OFF buttons
6. Device state will show in status column

## Troubleshooting

**Problem**: Controller not ready
- Check GPIO connections (GPIO 17 = TX, GPIO 23 = Power)
- Verify FreeRTOS task started (check logs for "RF433 task started")

**Problem**: Device not responding
- Verify correct protocol number (try 1-12)
- Increase repeat count (default: 4)
- Check TX power supply (GPIO 23 should be HIGH during transmission)
- Use a 433MHz receiver to sniff actual codes from remote

**Problem**: 401 Unauthorized
- Ensure you're logged in (JWT token present)
- Admin role required for all RF433 operations

## Power Optimization

The module uses power gating to minimize idle current:
- GPIO 23 controls power to the 433MHz transmitter
- Power is only enabled during transmission
- TX task yields when queue is empty
- Total idle overhead: ~472 bytes DRAM

## Future Enhancements

- [ ] RX support (code learning from remote)
- [ ] Protocol auto-detection
- [ ] Scheduling/automation integration
- [ ] Group commands (control multiple devices)
- [ ] Code database/presets
