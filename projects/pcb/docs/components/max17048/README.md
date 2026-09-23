# MAX17048

Fuel gauge dla ogniwa Li-Ion 1S.

## Uwagi
- naturalny kandydat na osobny modul `battery_gauge`,
- dla wariantu `MAX17048` pin `VDD` laczy sie bezposrednio z dodatnim biegunem ogniwa / `PACK+`,
- pin `CELL` pozostaje niepodlaczony w wariancie 1S `MAX17048`,
- `CTG` laczy sie do `GND`,
- `QSTRT` laczy sie do `GND`, jesli nie uzywasz hardware quick-start,
- wymagany jest tylko jeden zewnetrzny kondensator `0.1uF` miedzy `VDD` i `GND`,
- `ALRT` jest wyjsciem open-drain; jesli go uzywasz, system musi zapewnic pull-up,
- pull-upy `SDA/SCL` powinny nalezec do wspolnej magistrali I2C, nie do samego gauge,
- dla tego repo domyslny wariant to `system-side`, czyli gauge siedzi po stronie `VBAT_PROT / PACK+`, a nie na osobnym raw-side module,
- nie mieszaj go z ladowarka; to osobna funkcja systemu zasilania.

## Uwaga systemowa
- dokladnosc SoC zalezy od zgodnosci domyslnego modelu `ModelGauge` z rzeczywistym ogniwem,
- dla produktu docelowego warto przewidziec strojenie `RCOMP` i ewentualnie custom model baterii.
