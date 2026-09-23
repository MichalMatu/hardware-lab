# axp2101_pmic placement

## Zasady krytyczne
- `AXP2101` traktuj jako centralny element subsystemu zasilania, nie jako peryferium przy MCU
- `C_PMIC_BAT` musi siedziec maksymalnie blisko pinu `BAT`
- `C_PMIC_VBUSA` i `C_PMIC_VBUSB` musza siedziec maksymalnie blisko `VBUS`
- `C_PMIC_VREF` musi siedziec przy `VREF`
- `C_PMIC_SYSA` i `C_PMIC_SYSB` musza siedziec maksymalnie blisko `VSYS`
- `L_PMIC_SW` trzymaj przy pinie `SW`, a lokalny net `VSYS/AXP_PS` prowadz najpierw przez kondensatory, potem dalej w system
- petla bucka `LX1 -> L_DCDC1 -> C_DCDC1 -> GND` musi byc krotsza i ciasniejsza niz sygnaly sterujace
- `VIN1..4`, `ALDOIN` i `BLDOIN` powinny miec lokalne kondensatory przy wejsciu regulatorow
- `EP` i masa PMIC powinny miec pelny, niski-impedancyjny powrot do planu `GND`
- `EP` musi miec **minimum 9 thermal vias** do planu `GND` (zgodnie z `requirements.md` §7 i design guide). Powod: 4 buck DCDC w PMIC potrafia razem dac do `2A` przez kazdy LX i bez porzadnego termal-path uklad bedzie thermal-throttle przy pelnym obciazeniu. Grid `3x3` z odstepem `1.0 mm` na EP `3.4 x 3.4 mm` mieci sie z marginesem.

## Wymagania z design guide

### Trace widths (DG §5.1)
- `VBUS`, `VMID`, `BAT`, `VSYS`: **> 150 mil (3.81 mm)**
- kazdy `VIN*` DCDC i `LX*`: **> 150 mil**
- `ALDOIN`, `BLDOIN`, `DC1SW`, `DC4SW`: **> 150 mil**
- wyjscia LDO: szerokosc dobierana wedlug pradu obciazenia
- bateria umieszczona blisko punktu polaczenia, minimalna dlugosc tras, grube przewody by zmniejszyc DCR i drop napiecia

### VREF filtering i stackup (DG §5.2)
- `VREF` kondensator blisko pinu; jego punkt `GND` daleko od DCDC, by uniknac zaklocen.
- DCDC inductor blisko chipa, output cap blisko cewki; input cap blisko pinu wejsciowego, sciezka wejsciowa najpierw przez cap potem na pin (filter LC).
- **Stackup**: warstwa `GND` MUSI byc na warstwie sasiadujacej z warstwa komponentow. Jesli chip + cewki sa na top, `GND` ma byc na warstwie 2 (`In1.Cu`). Jesli chip + cewki sa na bottom (jak w HAT-cie, ze wzgledu na anteny host boarda), `GND` ma byc na warstwie 3 (`In2.Cu`).

### Dodatkowe zasady DCDC/LDO (DG §4.2)
- input/output kondensatory zgodne ze schematem typowej aplikacji
- nieuzywane LDO: output pin otwarty, brak output cap, default OFF
- `BLDOIN` ze zrodla DCDC: input cap `>= 4.7uF`; napiecie LDO musi byc nizsze niz DCDC source
- `DLDO1/DLDO2` default to switch (`DC1SW/DC4SW`), efuse moze przekonfigurowac na LDO
- `CPULDOS` input to `DCDC4`, napiecie config musi byc nizsze niz `DCDC4`
- kazdy DCDC: `1uH` inductor, saturation current > 30% load, internal R `<= 30 mohm`
- nieuzywany DCDC: `VIN` polaczony zgodnie ze schematem, `LX` i `FB` otwarte, brak output cap

## Zalecany porzadek na PCB
1. `U_PMIC`
2. `C_PMIC_VBUSA`, `C_PMIC_VBUSB`, `C_PMIC_BAT`, `C_PMIC_VREF`
3. `C_PMIC_SYSA`, `C_PMIC_SYSB`, `L_PMIC_SW`
4. `L_DCDC1`, `C_DCDC1`
5. lokalne kondensatory `VIN1..4`, `ALDOIN`, `BLDOIN`, `VRTC`
6. `R_PMIC_IRQ`, `R_PMIC_TS` i ewentualne pull-upy `I2C`

## Uwagi praktyczne
- wezly `VBUS`, `BAT`, `VSYS` i `SW` prowadz szerzej niz `I2C`
- trzymaj `SDA/SCK/IRQ` z dala od `LX*` i od cewki `SW`
- jesli dodasz front-end `USB + solar`, umieszczaj go przed `VBUS`, nie pomiedzy `VSYS` a PMIC
- `TS` traktuj jako net analogowy; nie prowadz go przy wezle bucka
- `PWRON` traktuj jako krytyczny sygnal startowy, nie gub go na dalekim zlaczu bez planu mechanicznego
