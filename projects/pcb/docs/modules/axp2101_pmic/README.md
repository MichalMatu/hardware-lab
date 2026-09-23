# axp2101_pmic

Modul dokumentacji dla zintegrowanego PMIC `AXP2101`.

## Status
- lokalny `design guide` oraz schematy `M5 CoreS3` i `Core2` zostaly przejrzane,
- domyslne zachowanie modulu zostalo zawazane do rzeczy potwierdzonych przez te referencje,
- `DCDC1` pozostaje jedynym domyslnie modelowanym wyjsciem systemowym,
- pomocnicze `DCDC` i `LDO` sa teraz jawnie opt-in,
- nadal nie jest to finalny sign-off produkcyjny bez potwierdzenia na realnych probkach `AXP2101`.

## Zakres
- wejscie `VBUS` / `VIN_5V` w zakresie `3.9V-5.5V`,
- ladowanie ogniwa `1S Li-Ion`,
- power-path `VBUS <-> VSYS <-> BAT`,
- fuel gauge i ADC po `TWSI/I2C`,
- glowna szyna `3V3` z `DCDC1`,
- referencyjny tor `SW -> L_PMIC_SW -> VSYS/AXP_PS`,
- sygnaly statusowe `IRQ`, `PWROK`, `CHGLED`,
- `TS` domyslnie jako `10k` do `GND`, o ile board nie dostarcza wlasnej sieci `NTC`,
- `VBackup` domyslnie zwiazany z `VRTC`, zgodnie z lokalnymi schematami,
- lokalne odsprzeganie dla `VREF`, `VBUS`, `VSYS`, `VMID`, `VRTC`, `VIN1..4`, `ALDOIN`, `BLDOIN`,
- pomocnicze wyjscia `DCDC2/3/4`, `ALDO1..4`, `BLDO1..2`, `CPULDOS`, `DLDO1/DC1SW`, `DLDO2/DC4SW` tylko wtedy, gdy board jawnie ich potrzebuje.

## Glowne komponenty
- `AXP2101`
- `L_PMIC_SW`
- `L_DCDC1`
- `C_PMIC_BAT`
- `C_PMIC_VBUSA`
- `C_PMIC_VBUSB`
- `C_PMIC_VREF`
- `C_PMIC_SYSA`
- `C_PMIC_SYSB`
- `C_DCDC1`
- `R_PMIC_IRQ`
- `R_PMIC_TS`

Elementy opcjonalne:
- `L_DCDC2`, `L_DCDC3`, `L_DCDC4`
- `C_DCDC2`, `C_DCDC3`, `C_DCDC4`
- `C_ALDO1..4`, `C_BLDO1..2`, `C_CPULDOS`, `C_DLDO1..2`
- `R_PMIC_SDA`, `R_PMIC_SCK`

## Kontrakt elektryczny
- wejscie zasilania PMIC: `V_USB` w biezacym frameworku
- wejscie baterii: `BAT_RAW_VCC`
- masa wspolna: `GND`
- glowna szyna wyjsciowa do MCU: `V_3V3`
- magistrala sterowania i telemetrii: `I2C0`
- opcjonalne sygnaly:
  - `AXP_IRQ`
  - `AXP_PWROK`
  - `AXP_CHGLED`
  - `BAT_TS`

## Decyzje projektowe
- `VIN1..4`, `ALDOIN` i `BLDOIN` sa zasilane z lokalnego `VSYS/AXP_PS` i maja lokalne `2.2uF`, zgodnie z `M5 CoreS3` i `Core2`,
- pin `SW` nie jest juz traktowany jako `NC`; obie lokalne referencje obsadzaja go przez `1uH` do lokalnego netu PMIC,
- nieuzywane `LDO` pozostaja otwarte bez output capacitor,
- nieuzywane `DCDC` maja podlaczone `VIN`, ale `LX` i `FB` pozostaja otwarte bez inductorow i output cap, zgodnie z `AXP2101设计指南_V1.0`,
- `IRQ` ma domyslnie `4.7k` pull-up (zgodnie z datasheet AXP2101 pin 38). Domena pull-up zalezy od boarda: HAT podpina go do `ESP_3V3` zeby host MCU widzial sygnal w domenie logicznej; designy ze stale-on logika typu RTC podpinaja do `VRTC` / `VBackup`. Module akceptuje `irq_pullup_rail` jako parametr,
- `TS` ma domyslnie `10k` do `GND`, jesli board nie dostarcza osobnego termistora lub sieci kwalifikacji temperatury,
- `VBackup` jest domyslnie zwiazany z `VRTC`, bo tak robia oba lokalne reference designy,
- `DLDO1/DC1SW`, `DLDO2/DC4SW` i `CPULDOS` nie sa traktowane jak "zwykle LDO"; ich sens zalezy od wybranego trybu PMIC i ustawien konfiguracyjnych,
- pull-upy `I2C` dla wariantu z `AXP2101` pozostaja `2.2k`, zgodnie z datasheetem.

## Mapowania z referencji
### M5 CoreS3
- `DCDC1 -> VDD_3V3`
- `DCDC3 -> VCC_3V3`
- `ALDO1 -> VDD_1V8`
- `ALDO2 -> VDDA_3V3`
- `ALDO3 -> VDDCAM_3V3`
- `ALDO4 -> VDD_3V3_SD`
- `BLDO1 -> AVDD`
- `BLDO2 -> DVDD`
- `DLDO1/DC1SW -> VCC_BL`
- `DCDC2`, `DCDC4`, `DLDO2/DC4SW`, `CPULDOS`, `GPIO1/FB5/RTCLDO2` pozostaja nieuzyte
- `IRQ` ma `10k` pull-up do `RTC_VDD`
- `TS` jest sciagane `10k` do `GND`
- `PWRON` i `PWROK` maja lokalne RC z `510R + 1nF`

### Core2
- `DCDC1 -> AXP_ESP`
- `DCDC3 -> AXP_BUS_3.3V`
- `BLDO1 -> AXP_BL`
- `DLDO1/DC1SW -> AXP_VIB`
- `VBackup` i `VRTC` sa zwiazane z `AXP_VRTC`
- domyslnie `IRQ` ma `10k` pull-up do `AXP_VRTC`, ale board-level override do logiki `3V3` przez `4.7k` jest sensowny, jesli host ma czytac `IRQ` bezposrednio
- `TS` nie jest pozostawione floating; referencja rozwiazuje je lokalnym rezystorem
- pozostale pomocnicze wyjscia sa uzywane selektywnie przez enable/jumper nets, wiec nie nadaja sie na bezwarunkowe defaulty modulu

## Ograniczenia biezacego szkicu
- brak finalnego modelu source mux / OR-ing dla `USB` i `solar`,
- brak modelu board-level RC dla `PWRON` i `PWROK`,
- nie modelujemy jeszcze `DCDC5 / GPIO1-FB5 / RTCLDO2`,
- najwieksze ryzyko pozostaje w factory defaults / startup sequence / napieciach wyjsciowych konkretnej partii PMIC,
- konfiguracja battery profile dla `E-gauge 3.0` pozostaje etapem bring-upu, a nie automatycznym elementem generatora.

## BOM modulu
- `U_PMIC`: `AXP2101`, obudowa `QFN-40 5x5`
- `L_PMIC_SW`: `1uH`, praktycznie molded power inductor `1210/3225` albo podobny, z niskim `DCR` (charge-pump dla `VMID`)
- `L_DCDC1`: `2.2uH` buck inductor dla glownej szyny `DCDC1`, praktycznie `1210/3225` albo podobny (wartosc z `AXP2101 Design Guide V1.0` i schematow M5)
- `C_PMIC_BAT`: `1uF` blisko `BAT`
- `C_PMIC_VBUSA`: `10uF`, praktycznie `0805` albo wiekszy
- `C_PMIC_VBUSB`: `2.2uF`
- `C_PMIC_VREF`: `1uF`
- `C_PMIC_SYSA`, `C_PMIC_SYSB`: `22uF` blisko `VSYS`, praktycznie `1206` albo wiekszy
- `C_DCDC1`: `22uF` na wyjsciu `DCDC1`, praktycznie `1206` albo wiekszy
- `C_PMIC_VIN1..4`: `2.2uF`
- `C_PMIC_ALDOIN`: `2.2uF`
- `C_PMIC_BLDOIN`: `2.2uF`
- `C_PMIC_VMID`: `2.2uF`
- `C_PMIC_VRTC`: `2.2uF`
- `R_PMIC_IRQ`: `10k`
- `R_PMIC_TS`: `10k`, jesli `TS` nie jest osobna siecia `NTC`
- `R_PMIC_SDA`, `R_PMIC_SCK`: `2.2k`, jesli pull-upy nie sa zapewnione w `i2c_bus`

Szczegoly BOM:
- [bom.md](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/bom.md)

## Layout
- `C_PMIC_BAT` trzymaj przy `BAT/GND`
- `C_PMIC_VBUSA` i `C_PMIC_VBUSB` trzymaj przy `VBUS/GND`
- `C_PMIC_VREF` trzymaj przy `VREF/GND`
- oba kondensatory `VSYS` trzymaj przy `VSYS/GND`
- `L_PMIC_SW` trzymaj przy pinie `SW` i przy lokalnym wezle `VSYS/AXP_PS`
- petla `LX1 -> L_DCDC1 -> C_DCDC1 -> GND` musi byc krotka i zwarta
- `IRQ`, `SDA`, `SCK` trzymaj z dala od wezlow `LX*`
- pad termiczny `EP` polacz solidnie z masa
- `VIN1..4`, `ALDOIN`, `BLDOIN` traktuj jako lokalne wejscia regulatorow, nie zostawiaj ich przypadkiem wiszacych

Szczegoly placementu:
- [placement.md](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/placement.md)

Checklista review:
- [review_checklist.md](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/review_checklist.md)

Bring-up:
- [bringup.md](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/bringup.md)

## Powiazany kod
- `library/modules/axp2101_pmic.py`
- `framework/module_registry.py`
- `library/interfaces.py`

## Generator
- modul zastepuje stos dyskretny (charger + power-path + LDO 3V3 + osobny fuel gauge) jednym ukladem,
- generator traktuje go jako modul produkujacy `V_3V3` na bazie `DCDC1` i udostepniajacy gauge przez I2C,
- gdy aktywny jest `axp2101_pmic`, generator ustawia `i2c_bus` z pull-upami `2.2k`,
- modul wymaga rownoczesnego wpisu `i2c_bus` w `[composition].modules`,
- pomocnicze wyjscia regulatorow trzeba wlaczyc jawnie przy wywolaniu `add_axp2101_pmic(...)`.

## Zrodla
- datasheet lokalny: [AXP2101_C3036461.pdf](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/references/AXP2101_C3036461.pdf)
- design guide: [AXP2101设计指南_V1.0.pdf](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/references/eggfly_AXP2101/AXP2101设计指南_V1.0.pdf)
- schemat referencyjny: [Sch_M5_CoreS3_v1.0.pdf](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/references/eggfly_AXP2101/Sch_M5_CoreS3_v1.0.pdf)
- schemat referencyjny: [Sch_Core2_v1.1_2023-07-20.pdf](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/references/eggfly_AXP2101/Sch_Core2_v1.1_2023-07-20.pdf)
- indeks referencji: [README.md](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/references/README.md)
- lokalne repo referencyjne: [eggfly_AXP2101](/Users/michal/Desktop/pcb/docs/modules/axp2101_pmic/references/eggfly_AXP2101/README.md)
