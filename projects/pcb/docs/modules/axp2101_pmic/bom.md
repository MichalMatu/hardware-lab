# axp2101_pmic BOM

## Rdzen modulu
- `U_PMIC`: `AXP2101`
- `L_PMIC_SW`: `1uH`, referencyjny link z pinu `SW` do lokalnego `VSYS/AXP_PS`; praktycznie molded power inductor `1210/3225` lub podobny. Rola: charge-pump dla `VMID`.
- `L_DCDC1`: `1uH` buck inductor dla glownej szyny `DCDC1` (3V3). Wartosc zgodna z `AXP2101 Design Guide V1.0 §4.2.6`: "kazdy DCDC uzywa 1uH cewki, current saturation > 30% load, internal R <= 30 mohm". Praktycznie molded power inductor `1210/3225` lub podobny.
- `C_PMIC_BAT`: `1uF`
- `C_PMIC_VBUSA`: `10uF`, praktycznie `0805` lub wiekszy
- `C_PMIC_VBUSB`: `2.2uF`
- `C_PMIC_VREF`: `1uF`
- `C_PMIC_SYSA`, `C_PMIC_SYSB`: `22uF`, praktycznie `1206` lub wiekszy
- `C_DCDC1`: `22uF`, praktycznie `1206` lub wiekszy
- `C_PMIC_VIN1`, `C_PMIC_VIN2`, `C_PMIC_VIN3`, `C_PMIC_VIN4`: `2.2uF`
- `C_PMIC_ALDOIN`: `2.2uF`
- `C_PMIC_BLDOIN`: `2.2uF`
- `C_PMIC_VMID`: `2.2uF`
- `C_PMIC_VRTC`: `2.2uF`

## Sygnaly pomocnicze
- `R_PMIC_IRQ`: `4.7k` jako default modulu, zgodnie z datasheet AXP2101 (Table 4-1 pin 38: "Connect the IRQ to a logic rail via a 4.7kΩ resistor"). Board-level mozna podpiac ten pull-up do `VRTC`, `VBackup` albo do `V_3V3` zaleznie od tego, ktory rail ma byc always-on dla logiki host MCU.
- `R_PMIC_TS`: `10k`, jesli `TS` nie jest osobna siecia `NTC`
- `R_PMIC_SDA`: `2.2k`, datasheet pin 39: "needs a 2.2kΩ Pull High"
- `R_PMIC_SCK`: `2.2k`, datasheet pin 40: "needs a 2.2kΩ Pull High"

## Aux rails tylko gdy naprawde sa uzywane
- `L_DCDC2`, `L_DCDC3`, `L_DCDC4`: `1uH` dla faktycznie wlaczonych pomocniczych buckow (default modulu, zgodnie z Design Guide §4.2.6).
- `C_DCDC2`, `C_DCDC3`, `C_DCDC4`: tylko dla faktycznie wlaczonych buckow
- `C_ALDO1..4`, `C_BLDO1..2`, `C_CPULDOS`, `C_DLDO1..2`: tylko dla faktycznie uzytych wyjsc

## Poza modulem
- front-end `USB + solar`, jesli oba zrodla maja isc na jedno wejscie `VBUS`
- `PWRON` i `PWROK` RC/button network, jesli board wymaga lokalnych przyciskow start/reset
- termistor / bateria `NTC` dla `TS`, jesli chcesz zachowac hardware qualification temperatury
- osobna ochrona ogniwa, jesli projekt nie zaklada polegania wylacznie na PMIC i ogniwie z `PCM`

## Footprint i package
- package: `QFN5*5-40-0.4` (X-Powers oznaczenie), wymiary z datasheet Figure 8-1:
  - body `D x E`: `4.90 / 5.00 / 5.10 mm` (min/nom/max)
  - EP `D2 x E2`: `3.30 / 3.40 / 3.50 mm` (min/nom/max)
  - pitch `e`: `0.40 mm BSC`
  - lead `b`: `0.15 / 0.20 / 0.25 mm`
  - height `A`: `0.80 / 0.85 / 0.90 mm`
- aktualny KiCad footprint: `VQFN-40-1EP_5x5mm_P0.4mm_EP3.5x3.5mm`. EP pad `3.5 x 3.5 mm` odpowiada MAX EP z datasheet i miesci nominalne `3.4 x 3.4 mm` z naddatkiem zgodnym z IPC-7351. Stencil paste rekomendowany podzielony (60-80% pokrycia EP w paru wycinkach), nie 1:1.
- thermal pad `EP` musi byc polaczony z system `GND` przez minimum 9 thermal vias (zgodnie z `requirements.md` §7) ze wzgledu na rozproszenie ciepla z buckow PMIC.

## Produkcja (z datasheet §8.3, §9)
- `MSL`: 3
- floor life out-of-bag: 168 h przy `<=30°C / 60%RH`
- reflow: lead-free, peak `240-250°C`, time above liquidus `60-90 s`
- bake jesli przekroczono floor life: `125°C / 8 h` w azocie

## Do doprecyzowania przy finalizacji
- decyzja jak modelowac `DCDC5 / RTCLDO2 / GPIO1-FB5`
- potwierdzenie battery profile dla `E-gauge 3.0`
