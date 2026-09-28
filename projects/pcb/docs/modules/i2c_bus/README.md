# i2c_bus

Maly reusable module wspolnej magistrali I2C.

## Zakres

- `SDA` i `SCL`,
- wspolne pull-upy do wybranej domeny logicznej,
- opcjonalny 4-pin breakout/header,
- bez sygnalow per-device typu `IRQ`, `ALRT` czy `INT`.

## Kod

`library/modules/i2c_bus.py`

## Kontrakt

Module przyjmuje `PowerDomain` i `I2CBus`. Domyslne pull-upy to `4.7k`, ale board moze je zmienic lub wylaczyc, jezeli magistrala ma juz poprawne podciaganie.

I2C nie jest zalezne od AXP2101. PMIC jest tylko jednym z mozliwych konsumentow tej magistrali.
