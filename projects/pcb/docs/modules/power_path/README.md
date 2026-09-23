# power_path

Modul dokumentacji dla przelaczania zasilania miedzy USB i bateria.

## Zakres
- load-sharing pomiedzy `V_USB` i bateria,
- generowanie wspolnej domeny `SYS_VCC`,
- ochrona przed backfeed i podstawowa selekcja zrodla.

## Powiazany kod
- `library/modules/power_path.py`
- `library/modules/battery_power.py`

## Kontrakt sieci
- wejscia: `V_USB`, `VBAT_PROT`
- wyjscie: `SYS_VCC`
- masa: `GND`

## Generator
- modul jest juz wpiety do `framework/module_registry.py`,
- obecnie wymaga modulu `battery_power`,
- przy wlaczonym `power_path` modul `power_3v3` bierze zasilanie z `SYS_VCC` zamiast bezposrednio z `V_USB`.

## Weryfikacja
- potwierdzic topologie PMOS + Schottky,
- potwierdzic spadki napiecia i warunki graniczne,
- sprawdzic zachowanie przy jednoczesnym USB i baterii,
- zweryfikowac czy docelowo potrzebny jest bardziej dopracowany ideal-diode / power-mux.
