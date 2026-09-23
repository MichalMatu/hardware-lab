# battery_power BOM

## Minimalny BOM
- `U_CHG`: `TP4056`
  footprint: `Package_SO:HSOP-8-1EP_3.89x4.89mm_P1.27mm_EP2.33x3.45mm`
- `R_PROG`: `1.2k`
  footprint: `Resistor_SMD:R_0603_1608Metric`
- `C_CHG_VCC`: `10uF`
  footprint: `Capacitor_SMD:C_0603_1608Metric`
- `C_CHG_BAT`: `1uF`
  footprint: `Capacitor_SMD:C_0603_1608Metric`
- `U_BMS`: `DW01A`
  footprint: `Package_TO_SOT_SMD:SOT-23-6`
- `R_BMS_VDD`: `470`
  footprint: `Resistor_SMD:R_0603_1608Metric`
- `C_BMS_VDD`: `0.1uF`
  footprint: `Capacitor_SMD:C_0603_1608Metric`
- `R_BMS_CS`: `2k`
  footprint: `Resistor_SMD:R_0603_1608Metric`
- `Q_BMS`: `FS8205A`
  footprint: `Package_TO_SOT_SMD:SOT-23-6`
- `R_PACK_POS`: `0R`
  footprint: `Resistor_SMD:R_0603_1608Metric`
  uwaga: wystepuje po to, by zachowac osobne nazwy domen `BAT_RAW_VCC` i `VBAT_PROT`

## Cel poszczegolnych elementow
- `TP4056`: realizuje ladowanie CC/CV ogniwa 1S
- `R_PROG`: ustala prad ladowania
- `C_CHG_VCC`: odsprzega wejscie ladowarki od `V_USB`
- `C_CHG_BAT`: stabilizuje pin `BAT` ladowarki
- `DW01A`: detekcja overcharge / overdischarge / overcurrent
- `R_BMS_VDD` + `C_BMS_VDD`: filtracja zasilania `DW01A`
- `R_BMS_CS`: rezystor pomiarowo-detekcyjny zgodny z typowym aplikacyjnym ukladem `DW01A`
- `FS8205A`: dwa MOSFET-y back-to-back na niskiej stronie baterii

## Opcjonalne sygnaly
- `CHRG` i `STDBY` z `TP4056` sa opcjonalne i moga byc pozostawione jako `NC`
- jesli pozniej zechcesz miec LED albo telemetrie stanu ladowania, mozna je wystawic jako osobne nety
