# Configuration Module

Centralny system konfiguracji aplikacji PlantStatus.

## Struktura

```
config/
├── AppConfig.h              # Główny aggregator (includes all domain configs)
├── HardwareConfig.h         # Piny GPIO i adresy hardware (HW, COM)
├── SensorTimingConfig.h     # Timery sensorów i logging (SENSOR, LOGGER, BTN)
├── NetworkConfig.h          # Network i API timeouty (NET, API)
├── RTCConfig.h              # RTC i synchronizacja czasu (RTC)
├── NotificationsConfig.h    # Telegram i powiadomienia (APP::NOTIFY)
├── PowerConfig.h            # Zarządzanie mocą i deep sleep (POWER)
├── SystemConfig.h           # System i factory reset (APP, FACTORY)
├── SensorConfig.h/cpp       # Kalibracja sensorów (runtime, NVS)
├── LoggingConfig.h/cpp      # Konfiguracja logowania (runtime, NVS)
└── SensorConfigHandlers.cpp # REST API handlers dla kalibracji
```

## AppConfig.h (Aggregator)

Główny plik includujący wszystkie moduły konfiguracyjne. Zapewnia **backward compatibility** - istniejący kod może nadal includować `AppConfig.h` i otrzyma dostęp do wszystkich namespace'ów.

Dla nowego kodu zalecane jest includowanie konkretnych plików:
```cpp
#include "config/HardwareConfig.h"    // Tylko piny GPIO
#include "config/SensorTimingConfig.h" // Tylko timery sensorów
```

## Domain-Specific Configs

### HardwareConfig.h
**Namespace: `HW`, `COM`**

Piny GPIO dla wszystkich peryferiów (I2C, sensory, przyciski, LED), adresy I2C i parametry komunikacji szeregowej.

### SensorTimingConfig.h
**Namespace: `SENSOR`, `LOGGER`, `BTN`**

- Parametry FreeRTOS task (stack size, priority, core)
- Interwały odczytów (READ_INTERVAL_MS, LOG_INTERVAL_MS)
- Parametry sensorów (liczba próbek, delaye stabilizacji)
- Konfiguracja data loggera (ścieżki, limity rozmiaru)
- Parametry przycisków (debounce, long press)

### NetworkConfig.h
**Namespace: `NET`, `API`**

Timeouty połączeń WiFi, HTTP API, filesystem mutex, timeouty sensorów przez API.

### RTCConfig.h
**Namespace: `RTC`**

Timeouty mutex dla DS3231, interwały synchronizacji NTP→RTC, parametry detekcji SNTP sync.

### NotificationsConfig.h
**Namespace: `APP::NOTIFY`**

Limity payload Telegram, timeouty TCP probe, socket timeouty dla HTTPS.

### PowerConfig.h
**Namespace: `POWER`**

Timeouty inactivity, grace period, interwały wake, limity walidacji API, countdown logging.

### SystemConfig.h
**Namespace: `APP`, `FACTORY`**

Metadane aplikacji (wersja, nazwa), main loop delay, parametry factory reset.

## SensorConfig

Dynamiczna kalibracja sensorów zapisywana w NVS (Preferences):

```cpp
struct SensorCalibration {
    float tempOffset;      // Offset temperatury
    float humidOffset;     // Offset wilgotności
    float luxOffset;       // Offset natężenia światła
    float soilOffset;      // Offset wilgotności gleby
    int soilMin, soilMax;  // Zakres ADC dla soil (kalibracja)
    int batAdcMin, batAdcMax; // Zakres ADC dla baterii
};
```

Uwaga: `batAdcMin/batAdcMax` kalibrują głównie **procent baterii** (mapowanie ADC → 0–100%).
Napięcie (`batVolt`) jest liczone osobno z surowego ADC w oparciu o stałe założenia o dzielniku (1:2) i referencji ADC.

### API Endpoints

Konfiguracja dostępna przez REST API (handlers w `SensorConfigHandlers.cpp`):

- `GET /rest/sensorConfig` - Pobierz aktualną kalibrację
- `POST /rest/sensorConfig` - Zapisz nową kalibrację

## Użycie

```cpp
#include "config/AppConfig.h"
#include "config/SensorConfig.h"

using namespace HW;
using namespace SENSOR;

void setup() {
    // Użycie AppConfig
    pinMode(USER_BUTTON, INPUT);
    Serial.begin(COM::SERIAL_BAUD_RATE);
    
    // Użycie SensorConfig
    SensorConfig::begin();
    auto& cal = SensorConfig::get();
    float calibratedTemp = rawTemp + cal.tempOffset;
}
```

## Migracja z rozproszonej konfiguracji

Przed:
```cpp
#define USER_BUTTON 35
#define SERIAL_BAUD_RATE 115200
constexpr uint32_t READ_INTERVAL_MS = 5000;
```

Po:
```cpp
using namespace HW;
using namespace COM;
using namespace SENSOR;
// USER_BUTTON, SERIAL_BAUD_RATE, READ_INTERVAL_MS dostępne bez define
```

## Zalety nowego podejścia

✅ **Single Source of Truth** - cała konfiguracja w jednym miejscu  
✅ **Type Safety** - `constexpr` zamiast `#define`  
✅ **Namespace** - brak konfliktów nazw, czytelna organizacja  
✅ **Łatwość zmiany** - modyfikacja wartości nie wymaga edycji wielu plików  
✅ **Runtime kalibracja** - SensorConfig w NVS z REST API  

## Uwagi

- `AppConfig.h` to header-only (tylko deklaracje `constexpr`)
- `SensorConfig` wymaga `SensorConfig::begin()` w `setup()`
- Zmiana wartości w `AppConfig.h` wymaga przebudowania firmware
- Zmiana wartości w `SensorConfig` działa runtime przez API
