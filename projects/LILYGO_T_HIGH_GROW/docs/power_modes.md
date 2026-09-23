# Power Modes & Endpoints

## Tryby pracy
- **always_on (always_on)**: Wi‑Fi + HTTP aktywne, logowanie co 5 min, usypianie po braku aktywności HTTP/przycisku.
- **battery_mode (battery_mode)**: brak Wi‑Fi/HTTP, cykl: wake (timer/RTC) → RTC→system time → single-shot read+log z on-demand zasilaniem sensorów → deep sleep.

## Konfiguracja trwała (Preferences)
- Namespace: `power_cfg`
- Klucze:
  - `offline` (bool, legacy key name): tryb battery_mode po restarcie (true = battery_mode, false = always_on).
  - `inact_ms` (uint): timeout bezczynności w trybie always_on (ms).
  - `grace_ms` (uint): okres ochronny po starcie, bez usypiania (ms).

## Endpoints
- `GET /rest/power/status` (authenticated)
  - Zwraca JSON: `{ wake_reason: "timer|button|other|unknown", sleep_requested: bool, sleep_eta_ms: uint, inactivity_timeout_ms: uint, grace_ms: uint, wake_interval_ms: uint, last_activity_ms: uint, uptime_ms: uint }`

## Inne zachowania
- Pre-sleep hook: przed deep sleep wywoływane `server.end()` i `esp32sveltekit.stop()` (best effort; podmienić na oficjalne API jeśli wymagane).
- On-demand zasilanie sensorów: `POWER_ON_DEMAND=true` (AppConfig) — POWER_CTRL HIGH tylko na czas odczytu/logu.
- Factory reset: przycisk >20 s ⇒ czyści Preferences (`sensor_cal`), formatuje LittleFS, restart.

## Przepływ battery_mode (wake z timera lub tryb ustawiony na battery_mode)
1) PowerConfig wczytuje konfigurację (battery_mode/inact/grace).
2) restoreSystemTimeOrFallback() z RTC.
3) singleShotReadAndLog() (power gating sensorów) → CSV.
4) Pre-sleep hook (stop serwer/Wi‑Fi) → deep sleep (timer 5 min, wake also na GPIO35 LOW).

## Przepływ always_on
1) Wi‑Fi/HTTP start, tasks aktywne.
2) Licznik aktywności resetowany przez HTTP i przycisk.
3) Po `grace_ms` zaczyna działać `inactivity_timeout_ms`; brak aktywności ⇒ requestSleep("inactivity").
4) Pre-sleep hook → deep sleep z tym samym timerem/wake button.

## Parametry domyślne (AppConfig)
- `POWER::OFFLINE_MODE_DEFAULT=false` (false = always_on, true = battery_mode)
- `POWER::INACTIVITY_TIMEOUT_MS=30000` (30 s)
- `POWER::GRACE_AFTER_BOOT_MS=30000` (30 s)
- `POWER::WAKE_INTERVAL_MS=300000` (5 min)

## Notatki do frontendu
- Status/heartbeat: poll `GET /rest/power/status` do wyświetlania ETA do sleep i `wake_reason`.
