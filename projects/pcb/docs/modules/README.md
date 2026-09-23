# docs/modules/

Dokumentacja na poziomie modulu, nie calej plytki.

## Struktura

- `docs/modules/<module_name>/README.md`
- `docs/modules/<module_name>/bom.md` opcjonalnie
- `docs/modules/<module_name>/placement.md` opcjonalnie
- `docs/modules/<module_name>/references/`

## Minimalna zawartosc kontraktu

- cel modulu,
- wejscia i wyjscia,
- nazwy sieci i poziomy napiec,
- wymagane komponenty,
- zaleznosci od innych modulow,
- krytyczne uwagi layoutowe,
- checklista walidacji z datasheetem.

## Zasada grupowania

- Grupuj dokumentacje wedlug funkcji modulu, a nie pojedynczego ukladu.
- Jesli kilka ukladow tworzy razem jeden blok funkcjonalny, trzymaj je w jednym module.

## Aktywne moduly

- `axp2101_pmic`: integrowany PMIC z charge, power-path, gauge i 3V3.
- `i2c_bus`: wspolna magistrala I2C dla AXP2101 i przyszlych peryferiow.
