# library/

Wspolna biblioteka wielokrotnego uzytku dla Growclip PCB.

## Aktualna zawartosc

- `interfaces.py` - lekkie kontrakty magistral i domen zasilania.
- `host_profile.py` - wspolna walidacja tozsamosci hosta i blokada uzycia profilu `draft` jako zweryfikowanej podstawy produkcyjnej.
- `board_profile.py` - walidacja cienkiego manifestu boardu oraz gate `production -> verified host`.
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

Nowy host musi przejsc kontrakt z `hosts/PROFILE_CONTRACT.md`; dopoki ma status `draft`, nie powinien byc traktowany jako podstawa produkcyjnego PCB. Manifest boardu przechodzi kontrakt z `boards/BOARD_CONTRACT.md`; status `production` wymaga hosta `verified`.
