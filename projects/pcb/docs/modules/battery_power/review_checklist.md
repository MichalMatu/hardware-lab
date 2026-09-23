# battery_power Review Checklist

Checklist do koncowego review schematu, layoutu i uruchomienia modulu `battery_power`.

## 1. Schematic Sign-Off
- `TP4056` ma poprawny pin map `TEMP / PROG / GND / VCC / BAT / STDBY / CHRG / CE / EP`
- `TEMP` jest zepniety do `GND`, jesli nie korzystasz z monitoringu temperatury baterii
- `R_PROG` odpowiada docelowemu pradowi ladowania
- `C_CHG_VCC` jest wpiety miedzy `V_USB` i `GND`
- `C_CHG_BAT` jest wpiety miedzy `BAT_RAW_VCC` i `GND`
- `DW01A` ma poprawny pin map `OD / CS / OC / NC / VCC / GND`
- `R_BMS_VDD >= 470` i `C_BMS_VDD >= 0.1uF`
- `R_BMS_CS` jest obecny i wpiety do strony `P-`
- `FS8205A` ma poprawna orientacje pinow `S1 / D1 / S2 / G2 / D2 / G1`
- `OC` i `OD` z `DW01A` ida na wlasciwe bramki `FS8205A`
- `BAT_RAW_VCC` i `VBAT_PROT` sa rozdzielone logicznie tylko po stronie dodatniej, a ochrona dzieje sie na stronie ujemnej

## 2. Layout Sign-Off
- `TP4056` ma sensowne pole miedzi dla odprowadzania ciepla
- `C_CHG_VCC` i `C_CHG_BAT` sa bardzo blisko `TP4056`
- sciezki glownego pradu baterii sa szersze niz sygnaly sterujace
- `DW01A` i `FS8205A` sa blisko siebie
- `B-` i `P-` nie zostaly przez przypadek zwarte w layoucie
- orientacja `FS8205A` zostala sprawdzona jeszcze raz po placement

## 3. System Integration
- `VBAT_PROT` jest uzywane przez reszte systemu jako rail baterii
- `BAT_RAW_VCC` jest uzywane tylko tam, gdzie rzeczywiscie trzeba widziec surowe napiecie ogniwa
- jesli w systemie jest `battery_gauge`, jest swiadoma decyzja czy ma siedziec po stronie raw czy protected
- `CHRG` / `STDBY` sa albo `NC`, albo swiadomie podlaczone do LED / MCU
- prad ladowania jest realistyczny dla termiki i zrodla `V_USB`

## 4. Bring-Up
- ladowarka startuje poprawnie po podaniu `V_USB`
- napiecie ogniwa rosnie zgodnie z oczekiwaniem
- `VBAT_PROT` zachowuje sie poprawnie przy obciazeniu
- ochrona reaguje na warunki fault zgodnie z oczekiwaniem
- `TP4056` nie wchodzi zbyt szybko w thermal throttling przy docelowym pradzie

## 5. Open Items Before Production
- finalnie potwierdzony prad ladowania i wartosc `R_PROG`
- potwierdzony producent i konkretny MPN `DW01A`
- potwierdzony producent i konkretny MPN `FS8205A`
- wykonany finalny review pinoutu `FS8205A` na poziomie footprint + assembly drawing
- wykonany finalny review DFM i termiki `TP4056`
