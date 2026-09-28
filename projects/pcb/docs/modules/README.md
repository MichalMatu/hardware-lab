# docs/modules/

Dokumentacja na poziomie modulu, nie calej plytki. Obecnosc dokumentacji nie oznacza automatycznie, ze istnieje gotowa implementacja w `library/modules/`.

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
- Dokumentacja moze wyprzedzac implementacje. Board moze wskazac modul dopiero, gdy odpowiadajacy mu plik istnieje w `library/modules/`.

## Aktualny kod reusable

- `i2c_bus` - **basic reusable**. Mala infrastruktura I2C z opcjonalnymi pull-upami i headerem.
- `axp2101_pmic` - **advanced optional**. Zachowany jako wartosciowy, zlozony blok PMIC, ale nie jest czescia domyslnej bazy Growclip i wymaga osobnego review elektrycznego/layoutowego przed uzyciem w nowym wariancie.

Pozostale katalogi w `docs/modules/` sa baza wiedzy i kandydatami do przyszlych reusable blocks, a nie automatycznie aktywnymi komponentami boardu.
