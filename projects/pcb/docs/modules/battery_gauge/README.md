# battery_gauge

Modul dokumentacji dla monitorowania stanu baterii.

## Status
- modul jest domkniety elektrycznie dla `MAX17048` w wariancie 1S i w repo jest traktowany jako `system-side`,
- implementacja w `library/modules/battery_gauge.py` uzywa juz poprawnego pin map i footprintu TDFN 2x2 mm,
- pozostaje tylko walidacja systemowa przy konkretnej baterii i firmware.

## Zakres
- fuel gauge dla ogniwa 1S,
- pomiar napiecia i SoC,
- interfejs I2C do MCU,
- dedykowany sygnal `ALRT` / interrupt nalezacy do modulu.

## Zaleznosci
- wymagany chroniony rail baterii `VBAT_PROT`,
- w obecnym generatorze modul zaklada wspolwystepowanie z `battery_power`,
- wspolna magistrala `I2C0` jest dostarczana przez osobny modul `i2c_bus`.

## Glowny komponent
- `MAX17048 / MAX17049`

## Kontrakt elektryczny
- zasilanie pomiarowe: `VBAT_PROT` / `PACK+`
- zasilanie logiki pull-up `ALRT`: `V_3V3` lub inny rail logiki hosta
- masa: `GND`
- magistrala: `I2C0_SDA`, `I2C0_SCL`
- sygnal dedykowany: `BAT_GAUGE_ALRT`

## BOM modulu
- `U_GAUGE`: `MAX17048G+` lub odpowiednik w obudowie `TDFN-8-1EP 2x2 mm`
- `C_GAUGE`: `0.1uF`, footprint `0603`, bezposrednio przy `VDD/GND`
- `R_GAUGE_ALRT`: `10k`, footprint `0603`, opcjonalny pull-up linii `ALRT`

Szczegoly BOM:
- [bom.md](/Users/michal/Desktop/pcb/docs/modules/battery_gauge/bom.md)

## Decyzje projektowe
- `VDD` w `MAX17048` jest jednoczesnie zasilaniem i pomiarem napiecia ogniwa 1S.
- w tym repo modul jest domyslnie `system-side`, czyli siedzi po stronie `VBAT_PROT / PACK+`.
- `CELL` pozostaje niepodlaczone dla `MAX17048`.
- `CTG` jest laczone do `GND`.
- `QSTRT` domyslnie jest laczone do `GND`; osobny net podawaj tylko gdy rzeczywiscie chcesz hardware quick-start.
- pull-upy `SDA` i `SCL` nie naleza do tego modulu; sa w `i2c_bus`.
- `ALRT` nalezy do tego modulu i ma lokalny pull-up do logiki hosta.

## Layout
- uklad trzymaj blisko wejscia `VBAT_PROT` / punktu `PACK+`
- `C_GAUGE` ustaw maksymalnie blisko pinow `VDD` i `GND`
- exposed pad polacz bezposrednio z polem `GND`
- linie `I2C` i `ALRT` sa mniej krytyczne niz polaczenie `BAT_RAW_VCC` do `VDD`

Szczegoly placementu:
- [placement.md](/Users/michal/Desktop/pcb/docs/modules/battery_gauge/placement.md)

Checklista review:
- [review_checklist.md](/Users/michal/Desktop/pcb/docs/modules/battery_gauge/review_checklist.md)

## Powiazany kod
- `library/modules/battery_gauge.py`
- `library/modules/peripherals.py` nie powinien juz byc rozszerzany o fuel gauge
- `library/modules/i2c_bus.py` jako osobny modul wspolnej magistrali

## Weryfikacja
- potwierdzone polaczenie `VDD -> VBAT_PROT / PACK+`, `GND/EP/CTG -> GND`, `CELL -> NC`
- potwierdzone `QSTRT -> GND`, jesli nieuzywany
- potwierdzone, ze system musi zapewnic pull-up dla `SDA`, `SCL` i `ALRT` jesli `ALRT` jest uzywany
- do walidacji systemowej pozostaje dopasowanie modelu baterii / `RCOMP` i progow alertow w firmware

## Generator
- modul jest juz wpiety do `framework/module_registry.py`,
- generator podpina go do `VBAT_PROT`, logiki `V_3V3` i wspolnej magistrali `I2C0`,
- `ALRT` jest tworzone wewnatrz modulu jako dedykowany sygnal `BAT_GAUGE_ALRT`,
- pull-up `ALRT` nalezy do modulu `battery_gauge`, a nie do wspolnej magistrali,
- generator wymaga obecnie obecnosci `battery_power`, zeby dostarczyc `VBAT_PROT`,
- obecnie `I2C0` jest routowane w generatorze do domyslnych pinow ESP32-S3 `IO8/IO9` jako konfiguracja startowa.

## Uwaga systemowa
- hardware modulu jest gotowy, ale dokladnosc SoC zalezy jeszcze od zgodnosci domyslnego modelu `ModelGauge` z wybranym ogniwem.
- dla produktu docelowego warto przewidziec strojenie `RCOMP` i ewentualnie wgranie custom modelu baterii zgodnie z dokumentacja Maxim/ADI.
