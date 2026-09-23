# rtc

Modul dokumentacji dla zegara czasu rzeczywistego.

## Zakres
- RTC na magistrali I2C,
- podtrzymanie czasu,
- opcjonalne wyjscie alarmu / interrupt.

## Glowny komponent
- `PCF8563`

## Powiazany kod
- obecnie czesciowo zahaczony w `library/modules/peripherals.py`

## Weryfikacja
- potwierdzic wymagania kwarcu i polaczen,
- ustalic czy potrzebna jest bateria podtrzymujaca,
- potwierdzic wymagania pull-up dla I2C i ewentualnego INT.
