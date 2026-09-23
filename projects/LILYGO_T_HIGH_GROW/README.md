# LILYGO T-HIGrow / PlantStatus

ESP32 firmware and SvelteKit web UI for the LILYGO TTGO T-HIGrow plant sensor. This is a sanitized snapshot migrated into Hardware Lab.

## Current capabilities
- DHT11 and BH1750 plus onboard soil moisture/salt and battery ADC readers
- LittleFS logging with binary history and chart API
- Wi-Fi AP/STA configuration, NTP and RTC synchronization
- Power/deep-sleep management and button wake handling
- RF433 device control
- Telegram notifications over TLS
- Embedded SvelteKit web interface

## Build
```sh
cp secrets.example.ini secrets.ini
pio run
```
`secrets.ini` is ignored by Git. Add Telegram credentials only to the local copy.

## Migration/security note
The former standalone repository contained a committed Telegram Bot API token. Its Git history was therefore not merged into `hardware-lab`. This directory is a sanitized snapshot of source commit `812779ba298814a20443a90edd00f707a8212af5`. The exposed bot token must be rotated separately in BotFather.

## Old README images
The previous README referenced `images/PlantStatus.png`, `images/charts.png` and related screenshots, but the final `main` branch no longer contained an `images/` directory. Those links were broken and represented an older UI, so they were removed rather than restored.
