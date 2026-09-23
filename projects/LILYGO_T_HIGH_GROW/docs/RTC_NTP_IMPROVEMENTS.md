# Poprawki integracji RTC/NTP

**Data**: 2025-12-14

## Wprowadzone zmiany

### 1. Zabezpieczenie endpoint `/rest/rtc/sync` ✅

- **Problem**: Endpoint dostępny publicznie bez autoryzacji
- **Rozwiązanie**: 
  - Dodano `SecurityManager` do `SimpleSensorService`
  - Endpoint wymaga uprawnień administratora (`IS_ADMIN`)
  - Walidacja system time przed zapisem do RTC (zakres 2025-2040)
  - Zwracane klarowne komunikaty błędów w JSON

**Pliki**: [src/sensors/SimpleSensorService.h](../src/sensors/SimpleSensorService.h), [src/sensors/SimpleSensorService.cpp](../src/sensors/SimpleSensorService.cpp)

### 2. Poprawa logiki synchronizacji w RTCTask ✅

- **Problem**: 
  - Potencjalnie ciągłe zapisy RTC przy `SNTP_SYNC_STATUS_COMPLETED`
  - Brak zapisu RTC gdy makro nie jest dostępne
  - Sprawdzanie co 1s obciąża CPU
  
- **Rozwiązanie**:
  - Dodano `lastKnownSntpTime` do wykrywania nowych synchronizacji
  - Throttling zapisów RTC: max raz na 60s (`kMinSyncIntervalMs`)
  - Fallback dla systemów bez `SNTP_SYNC_STATUS_COMPLETED` (wykrywanie zmian czasu)
  - Zwiększono interwał pętli do 5s (mniej obciążenie CPU)
  - Tracking `lastSyncTime` w RTCModule dla diagnostyki

**Pliki**: [src/hardware/RTCTask.cpp](../src/hardware/RTCTask.cpp), [src/hardware/RTCModule.cpp](../src/hardware/RTCModule.cpp), [src/hardware/RTCModule.h](../src/hardware/RTCModule.h)

### 3. Walidacja payload `/rest/time` ✅

- **Problem**: Backend akceptował dowolny rok, frontend blokował się po ustawieniu nieprawidłowej daty
  
- **Rozwiązanie**:
  - Walidacja zakresu lat 2025-2040 w `NTPSettingsService::configureTime`
  - Sprawdzenie czy NTP jest wyłączony przed ustawieniem manualnego czasu
  - Zwracane klarowne komunikaty błędów w JSON z kodem statusu
  - Lepsze logowanie operacji

**Pliki**: [lib/framework/network/NTPSettingsService.cpp](../lib/framework/network/NTPSettingsService.cpp)

### 4. Nowy endpoint `/rest/rtc/status` ✅

- **Funkcjonalność**: Zwraca status RTC w formacie JSON
- **Zwracane dane**:
  - `initialized`: czy RTC jest zainicjowany
  - `lost_power`: czy moduł stracił zasilanie (bateria rozładowana)
  - `rtc_time`: aktualny czas z RTC (ISO8601 UTC)
  - `rtc_unix`: czas RTC jako Unix timestamp
  - `last_sync_unix`: timestamp ostatniej synchronizacji
  - `last_sync_time`: data ostatniej synchronizacji lub "never"
  
- **Autoryzacja**: Wymaga uwierzytelnienia (`IS_AUTHENTICATED`)

**Pliki**: [src/sensors/SimpleSensorService.cpp](../src/sensors/SimpleSensorService.cpp), [src/hardware/RTCModule.cpp](../src/hardware/RTCModule.cpp), [src/hardware/RTCModule.h](../src/hardware/RTCModule.h)

### 5. Ulepszenia UI ✅

**Dodano kartę "RTC Status (DS3231)"** wyświetlającą:
- Status baterii backup (OK / Lost power warning)
- Aktualny czas z RTC
- Timestamp ostatniej synchronizacji

**Poprawiono komunikaty**:
- "Time settings updated successfully" zamiast mylącego "Security settings updated"
- Parsowanie błędów JSON z backendu
- Lepsze komunikaty przy błędach ustawiania czasu

**Pliki**: [interface/src/routes/connections/ntp/NTP.svelte](../interface/src/routes/connections/ntp/NTP.svelte), [interface/src/lib/types/models.ts](../interface/src/lib/types/models.ts)

## Przepływ działania

### Start urządzenia
1. `RTCTask::begin()` inicjuje DS3231
2. Sprawdzany jest `lostPower()` - jeśli bateria rozładowana, ustawia fallback 2025-01-01
3. Przywracany jest system time z RTC (jeśli prawidłowy)
4. Fallback na 2025-01-01 jeśli RTC ma nieprawidłowy czas

### Synchronizacja NTP
1. Framework uruchamia SNTP gdy WiFi się połączy
2. `RTCTask::taskLoop()` sprawdza co 5s czy nastąpił nowy sync
3. Po wykryciu sync i walidacji system time → zapisuje do RTC
4. Throttling zapewnia max 1 zapis na 60s

### Manualne ustawienie czasu
1. UI wyłącza NTP i wysyła `POST /rest/time` z lokalnym czasem
2. Backend waliduje format i zakres lat (2025-2040)
3. System time jest aktualizowany
4. UI wywołuje `POST /rest/rtc/sync` (3 próby z retry)
5. Endpoint waliduje system time i zapisuje do RTC

### Monitoring (UI)
- Co 5s pobierane są `/rest/ntpStatus` i `/rest/rtc/status`
- Wyświetlane są informacje o czasie systemowym i stanie RTC
- Alerty gdy bateria RTC jest rozładowana

## Testy zalecane

1. **Kompilacja**: `pio run -e esp32dev_test` ✅
2. **Upload i monitor**: `pio run -e esp32dev_test -t upload && pio device monitor`
3. **UI**: Sprawdzić kartę RTC Status, próbę manualnego ustawienia czasu
4. **Security**: Próba wywołania `/rest/rtc/sync` bez tokenu (powinno zwrócić 401/403)
5. **Walidacja**: Próba ustawienia roku 2100 via `/rest/time` (powinno zwrócić 400)

## Uwagi

- Stała `NET::NTP_SYNC_INTERVAL_MS` w [src/config/AppConfig.h](../src/config/AppConfig.h) jest nieużywana - do usunięcia lub integracji z LWIP SNTP
- Przy braku baterii CR2032 w DS3231, `lost_power` będzie zawsze `true` - to normalne zachowanie
- Makro `SNTP_SYNC_STATUS_COMPLETED` powinno być dostępne w ESP-IDF >= 4.4, fallback dla starszych wersji działa poprawnie

### Tryb bateryjny / deep sleep

- Po wybudzeniu wykonujemy świeży odczyt sensorów (brak cache snapshotów między sesjami).
- CSV loguje także wpisy z błędami jako `NaN`, aby widać było momenty problemów.
- API `/api/sensors` zwraca ostatni snapshot z NaN oraz metadane: `lastGoodSeq/timestamp_ms` i `lastErrorInfo` z czasem ostatniego błędu.
