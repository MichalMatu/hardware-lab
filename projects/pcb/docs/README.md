# docs/

Source of Technical Truth dla aktywnego zakresu projektu: HAT pod `ESP32-DevKitC V4` zbudowany wokol PMIC `AXP2101`.

## Struktura

- `docs/modules/<module_name>/README.md`: kontrakt modulu, wejscia/wyjscia, ograniczenia i checklista walidacji.
- `docs/modules/<module_name>/references/`: datasheety i app notes dla danego modulu.
- `docs/components/<component_name>/README.md`: notatki dla konkretnego ukladu, jesli datasheet nie wystarcza.
- `docs/reference_designs/`: materialy referencyjne (gotowe plytki, devkity, decyzje kompatybilnosci).

## Biezacy podzial referencji

- `docs/modules/axp2101_pmic/`: PMIC AXP2101 z power-path, charge, gauge i 3V3 z DCDC1.
- `docs/modules/i2c_bus/`: wspolna magistrala I2C dla AXP2101 i przyszlych peryferiow HAT-a.
- `docs/components/axp2101/`: notatki do samego ukladu AXP2101.
- `docs/reference_designs/esp32_hat/`: decyzja kompatybilnosci i pinout DevKitC V4 dla HAT-a.

Pozostala dokumentacja (TP4056/DW01A/FS8205A, MAX17048, MT3608, AP2112, PCF8563, ESP32-S3, profile sensorow itd.) zostala usunieta razem z odpowiadajacymi modulami. Mozna ja przywrocic z gita, gdy odpowiedni modul wroci do projektu.
