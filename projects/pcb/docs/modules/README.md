# docs/modules/

Ten katalog dokumentuje **wylacznie aktywne reusable modules**, czyli moduly posiadajace implementacje w `library/modules/`.

## Aktywne moduly

- `i2c_bus` - maly, podstawowy klocek wspolnej magistrali I2C.
- `axp2101_pmic` - zaawansowany i opcjonalny blok PMIC; nie jest domyslnym elementem Growclip.

## Regula

Dla `library/modules/<name>.py` powinien istniec `docs/modules/<name>/README.md` i odwrotnie. Kandydat na przyszly modul pozostaje w historii Gita albo jest odtwarzany dopiero wtedy, gdy wraca do aktywnego projektu.

Dokumentacja modulu opisuje kontrakt, ograniczenia, layout/bring-up i kuratorowane zrodla. Nie trzymamy tutaj surowych paczek vendorow ani przypadkowych reference designow.
