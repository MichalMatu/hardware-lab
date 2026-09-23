# axp2101_pmic bring-up

To jest checklista uruchomieniowa dla pierwszych probek z `AXP2101`.

## Krytyczny fakt
- `AXP2101` ma startup sequence i default voltages oparte o `customization / EFUSE`.
- Nie wolno zakladac w ciemno, ze `DCDC1` po starcie ma `3.3V`, dopoki nie zostanie to potwierdzone na realnej probce.

## Pierwsze pomiary bez MCU
1. Podaj stabilne `5.0V` na `VBUS`.
2. Sprawdz `VBUS`, `VSYS`, `BAT`, `DCDC1`.
3. Potwierdz, czy `DCDC1` rzeczywiscie startuje i jakie ma napiecie.
4. Potwierdz, czy `PWRON` jest wymagany przy starcie tylko z baterii.

## Pierwsze odczyty po I2C
- status `VBUS_GOOD`
- rejestry `VINDPM` i `IINLIM`
- rejestry chargera `ICHG`, `IPRECHG`, `VREG`
- `VBAT`, `VBUS`, `VSYS`
- procent baterii z `E-gauge`

## Ustawienia, ktore trzeba jawnie zweryfikowac
- docelowe napiecie `DCDC1`
- `VINDPM` dla wybranego panelu / zrodla 5V
- `IINLIM` zgodne z realnym zrodlem
- `ICHG` zgodne z ogniwem i termika
- progi low-battery IRQ

## Testy funkcjonalne
- start tylko z `VBUS`
- start tylko z baterii
- przejscie `VBUS -> bateria`
- przejscie `bateria -> VBUS`
- ladowanie przy niskiej i sredniej baterii
- reakcja `IRQ` na low-battery i charger events

## Uwaga: GPIO27 / PWRON_BTN po stronie host boarda
- topologia HAT-a: `PMIC PWRON pin <- 510R <- ESP_GPIO27 <- SW_PWRON -> GND`, z `1nF` na PWRON do GND.
- przy puszczonym przycisku linia `PWRON` plywa do wewnetrznego pull-up PMIC do `VRTC` (~1.8V). To napiecie poprzez `510R` jest tez na `ESP_GPIO27`, czyli ponizej `VIH` ESP32 (~2.475V dla `3V3`).
- konsekwencja: ESP NIE moze odczytac stanu przycisku w trybie czystego `INPUT` (poziom niedeterministyczny). Wlasciwe uzycia:
  - `INTERRUPT` na falling edge na `GPIO27` jako "user pressed PWRON",
  - `OUTPUT_LOW` na `GPIO27` jako software-driven power-off (zwiera PWRON do GND przez 510R).
- nie podlaczac dodatkowego pull-up na `GPIO27` po stronie HAT-a: zaburzy logike soft power-off i moze zmienic timing debounce wewnatrz PMIC.

## Testy pod panel solarny
- zmierzyc `Voc` panelu bez obciazenia
- potwierdzic, ze `Voc` nie przekracza `5.5V` na `VBUS`
- ustawic `VINDPM` tak, by zrodlo sie nie zapadalo
- sprawdzic zachowanie przy slabym swietle i skokach obciazenia systemu
