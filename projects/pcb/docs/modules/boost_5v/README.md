# boost_5v

Modul dokumentacji dla przetwornicy podnoszacej napiecie z baterii do 5 V.

## Zakres
- step-up z `V_BAT` do `V_5V`,
- dobor cewki, diody i dzielnika sprzezenia zwrotnego,
- ograniczenia pradowe i sprawnosc.

## Glowny komponent
- `MT3608`

## Powiazany kod
- `library/modules/boost_5v.py`
- `library/modules/boost.py` jako warstwa kompatybilnosci

## Weryfikacja
- potwierdzic dzielnik FB dla docelowego napiecia wyjsciowego,
- potwierdzic dobor cewki i diody,
- oszacowac prad szczytowy i straty cieplne,
- potwierdzic minimalny layout petli przelaczajacej.
