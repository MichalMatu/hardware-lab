# library/modules/

Kod w tym katalogu ma byc faktycznie reusable pomiedzy roznymi boardami Growclip.

## Zasady

- Modul nie zna konkretnego hosta ani numerow jego GPIO.
- Wejscia/wyjscia dostaje przez `Net`, `PowerDomain`, `I2CBus` lub podobny jawny kontrakt.
- Modul nie ustala obrysu plytki, pozycji headerow hosta ani antenna keep-out.
- Modul nie jest automatycznie wlaczany do boardu tylko dlatego, ze istnieje.
- Board wybiera modul jawnie w swoim manifeście.
- Zlozony modul zasilania/RF/USB wymaga osobnego review przed uzyciem produkcyjnym.

## Aktualne moduly

### `i2c_bus.py`
Status: **basic reusable**.

Dodaje opcjonalne pull-upy i opcjonalny breakout header dla przekazanej magistrali I2C.

### `axp2101_pmic.py`
Status: **advanced optional**.

Zachowany jako wartosciowy blok i punkt wyjscia dla wariantu z bateria/PMIC. Nie jest domyslnym elementem `growclip_*_basic`. Przed ponownym uzyciem wymaga audytu konfiguracji PMIC, power path, elementow pasywnych i layoutu dla konkretnego wariantu.
