# library/

Wspolna biblioteka wielokrotnego uzytku dla plytek w repo.

## Zawartosc

- `interfaces.py`: kontrakty pomiedzy modulami (np. `PowerDomain`, `I2CBus`).
- `modules/axp2101_pmic.py`: blok PMIC AXP2101 (charge, power-path, gauge, 3V3 z DCDC1).
- `modules/i2c_bus.py`: wspolna magistrala I2C z pull-upami i headerem breakout.

W repo zostala dzis tylko jedna aktywna plytka (`esp32_devkitc_hat`) i biblioteka jest przyciecia do modulow ktore ta plytka realnie uzywa. Pozostale moduły (battery_power, power_path, sensors, usb itd.) zostaly usuniete w ramach cleanupu; mozna je przywrocic z gita, gdy wroca razem z nowa plytka.

## Workflow rozszerzenia

1. Dodaj implementacje w `library/modules/<module_name>.py`.
2. Opisz kontrakt i ograniczenia w `docs/modules/<module_name>/README.md`.
3. Dopisz wpis do `framework/module_registry.py`, jesli modul ma byc skladany z `board.toml`.
