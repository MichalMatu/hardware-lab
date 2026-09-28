# library/

Wspolna biblioteka wielokrotnego uzytku dla Growclip PCB.

## Aktualna zawartosc

- `interfaces.py` - lekkie kontrakty magistral i domen zasilania.
- `modules/axp2101_pmic.py` - reusable blok AXP2101.
- `modules/i2c_bus.py` - reusable infrastruktura I2C.
- `kicad/` - male, neutralne helpery do automatyzacji `pcbnew`.

AXP2101 jest opcjonalnym modulem. Obecnosc implementacji w bibliotece nie oznacza, ze ma byc uzywana w kazdym boardzie.

## Kiedy dodawac modul

Dodaj implementacje do `library/modules/`, gdy funkcja:
- ma zastosowanie w wiecej niz jednym wariancie,
- ma jasno zdefiniowany kontrakt,
- nie zalezy od mechaniki konkretnego hosta,
- posiada dokumentacje lub reference w `docs/modules/`.

Do `library/kicad/` trafiaja tylko helpery niezalezne od konkretnego boardu. Placementy, obrysy i opisy zalezne od hosta pozostaja w profilu hosta albo aktywnym boardzie.

Mechanika hosta trafia do `hosts/`. Polaczenia specyficzne tylko dla jednego produktu zostaja w `boards/<board>/`.
