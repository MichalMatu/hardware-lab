# AXP2101

`AXP2101` to zaawansowany PMIC dla systemow 1S Li-Ion: charger, power-path/NVDC, telemetry/ADC, fuel gauge oraz wiele regulatorow buck/LDO.

## Rola w Growclip

AXP2101 jest **opcjonalnym** komponentem dla wariantow wymagajacych rozbudowanego zasilania bateryjnego. Nie jest elementem minimalnego HAT-a i nie jest globalnym zalozeniem architektury.

## Najwazniejsze ograniczenia

- pojedyncze wejscie `VBUS`; dodatkowe zrodla zasilania wymagaja osobnego front-endu,
- konfiguracja startup/factory defaults i profil E-gauge musza byc potwierdzone na realnym hardware,
- `PWRON`, `PWROK`, `TS`, domena pull-up `IRQ` i battery protection sa decyzjami systemowymi, nie uniwersalnymi defaultami,
- QFN-40 5x5 i przetwornice impulsowe wymagaja starannego layoutu.

Aktywny kontrakt: `../../modules/axp2101_pmic/README.md`.
Datasheet: `../../modules/axp2101_pmic/references/AXP2101_C3036461.pdf`.
