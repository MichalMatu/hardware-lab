# power_3v3

Modul dokumentacji dla glownego regulatora 3.3 V.

## Zakres
- konwersja z `V_USB` lub `SYS_VCC` do `V_3V3`,
- odsprzeganie wejscia i wyjscia,
- stabilnosc regulatora i budzet cieplny.

## Glowny komponent referencyjny
- `AP2112K-3.3`

## Powiazany kod
- `library/modules/power_3v3.py`
- `library/modules/power.py` jako warstwa kompatybilnosci

## Uwagi
- Implementacja w `library/modules/power_3v3.py` nadal uzywa placeholdera `AP1117-33`.
- Przed produkcyjnym użyciem trzeba zgrac implementacje i dokumentacje na jeden rzeczywisty regulator.

## Generator
- bez `power_path` regulator bierze zasilanie z `V_USB`,
- z `power_path` regulator bierze zasilanie z `SYS_VCC`.
