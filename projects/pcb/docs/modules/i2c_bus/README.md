# i2c_bus

Modul dokumentacji dla wspolnej magistrali I2C.

## Zakres
- wspolne pull-upy dla `SDA` i `SCL`,
- opcjonalny breakout/header dla magistrali,
- logiczny punkt integracji dla peryferiow I2C,
- bez per-device sygnalow typu `ALRT` lub `INT`.

## Powiazany kod
- `library/modules/i2c_bus.py`

## Kontrakt
- zasilanie logiczne: `V_3V3`
- sygnaly: `I2C0_SDA`, `I2C0_SCL`
- masa: `GND`

## Generator
- modul jest osobnym wpisem w `framework/module_registry.py`,
- modul jest wymagany przez `axp2101_pmic` (sterowanie i telemetria PMIC),
- gdy aktywny jest `axp2101_pmic`, generator ustawia pull-upy `2.2k` zamiast domyslnych `4.7k`,
- sygnaly peryferyjne poza `SDA` i `SCL` pozostaja odpowiedzialnoscia konkretnego modulu konsumujacego magistrale.
