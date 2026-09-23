# PCF8563

RTC na I2C.

## Uwagi
- kandydat na osobny modul `rtc`,
- sprawdzic wymagania kwarcu i ewentualnego sygnalu alarmowego,
- nie wrzucac do jednego worka z `battery_gauge`, bo to odrebna odpowiedzialnosc.
