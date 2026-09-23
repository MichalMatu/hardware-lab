# AXP2101

Notatki do ukladu `AXP2101`.

## Co daje ten uklad
- `1S Li-Ion` charger
- power path / `NVDC`
- `TWSI/I2C`
- `E-gauge 3.0`
- ADC dla `VBAT`, `VBUS`, `VSYS`
- wiele buckow i LDO

## Dlaczego trafia do repo
- jeden uklad obsluguje cala sciezke zasilania `5V / 500mA` HAT-a: ladowanie 1S, power-path, regulacja 3V3 i fuel gauge,
- eliminuje stos `TP4056 + DW01A + FS8205A + LDO + MAX17048`, ktory wczesniej byl realizowany jako osobne moduly.

## Najwazniejsze ograniczenia
- ma jedno wejscie zasilania `VBUS`, wiec `USB + solar` wymagaja osobnego front-endu,
- wymaga swiadomego podejscia do `PWRON`, szczegolnie przy starcie tylko z baterii,
- to nie jest automatyczny zamiennik protection IC dla golej celi 18650,
- obudowa `QFN-40 5x5` jest istotnie trudniejsza od prostych ladowarek typu `CN3065`.

## Zrodla
- [AXP2101 datasheet](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/references/AXP2101_C3036461.pdf)
- https://www.lcsc.com/product-detail/C3036461.html
