# axp2101_pmic

Zaawansowany, **opcjonalny** reusable module dla wariantow Growclip wymagajacych zasilania 1S Li-Ion z PMIC AXP2101. Nie jest czescia minimalnej bazy HAT-a.

## Status

Kod `library/modules/axp2101_pmic.py` modeluje podstawowy power-path, interfejs sterowania oraz DCDC1. Modul jest przydatnym punktem startowym, ale wymaga hardware bring-up przed produkcyjnym sign-off.

## Zakres domyslny

- `VBUS`, `BAT`, `VSYS`, wspolna masa,
- I2C/TWSI i `IRQ`,
- `PWROK`, `PWRON`, `CHGLED`, `TS`, `VRTC/VBackup`,
- lokalne odsprzeganie krytycznych domen,
- `DCDC1` jako glowna konfigurowalna szyna,
- pozostale DCDC/LDO tylko jawnie opt-in.

## Swiadome ograniczenia

- brak uniwersalnego source mux / OR-ing,
- battery protection pozostaje decyzja boardu,
- RC/button network dla `PWRON/PWROK` pozostaje board-specific,
- `GPIO1/FB5/RTCLDO2` nie jest automatycznie konfigurowany,
- startup defaults, docelowe napiecia i E-gauge wymagaja potwierdzenia na konkretnej partii/konfiguracji PMIC,
- domena pull-up `IRQ` musi byc dobrana do architektury danego boardu.

## Layout

Najkrotsze mozliwe petle dla `LX`, cewek i kondensatorow wyjsciowych; kondensatory wejsc/wyjsc bezposrednio przy pinach; solidny GND/EP; I2C i IRQ z dala od wezlow switching.

Szczegoly:

- [BOM](bom.md)
- [placement](placement.md)
- [review checklist](review_checklist.md)
- [bring-up](bringup.md)

## Kod

- `library/modules/axp2101_pmic.py`
- `library/interfaces.py`

## Kuratorowane zrodla

- [AXP2101 datasheet v1.4](references/AXP2101_C3036461.pdf)
- [AXP2101 Design Guide v1.0](references/AXP2101_Design_Guide_V1.0.pdf)
- [M5 CoreS3 reference schematic](references/M5_CoreS3_reference.pdf)
- [M5 Core2 reference schematic](references/M5_Core2_reference.pdf)
- [reference index](references/README.md)
