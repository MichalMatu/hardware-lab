# battery_power

Modul dokumentacji dla ladowania i ochrony ogniwa Li-Ion 1S.

## Status
- implementacja w `library/modules/battery_power.py` modeluje teraz referencyjny blok `TP4056 + DW01A + FS8205A`,
- modul ma jawne pin mapy dla wszystkich trzech ukladow i nie polega juz na niepewnych nazwach symboli bibliotecznych,
- pozostaje walidacja systemowa dla konkretnej plytki i finalnego doboru pradu ladowania.

## Zakres
- ladowanie ogniwa 1S z `V_USB`,
- ochrona overcharge / overdischarge / overcurrent przez `DW01A + FS8205A`,
- wydzielenie surowego raila ogniwa `BAT_RAW_VCC`,
- wydzielenie chronionego raila systemowego `VBAT_PROT`.

## Glowne komponenty
- `TP4056`
- `DW01A`
- `FS8205A`

## Kontrakt elektryczny
- wejscie ladowarki: `V_USB`
- masa systemowa / pack negative: `GND`
- surowy plus ogniwa: `BAT_RAW_VCC`
- chroniony plus packa: `VBAT_PROT`
- wewnetrzny raw negative ogniwa: `BAT_RAW_GND` przy wlaczonej ochronie

## Decyzje projektowe
- `TP4056` laduje bezposrednio `BAT_RAW_VCC`
- `DW01A + FS8205A` odcinaja strone ujemna baterii, nie dodatnia
- `VBAT_PROT` jest dodatnim wyjsciem packa i pozostaje polaczone z `BAT_RAW_VCC`
- rozdzielenie `B-` i `P-` jest modelowane wewnatrz modulu przy wlaczonej ochronie
- `CHRG` i `STDBY` sa opcjonalne; jesli ich nie uzywasz, pozostaja `NC`

## Ograniczenie architektury
- obecny framework wystawia na poziomie boarda glownie dodatnie domeny zasilania,
- dlatego low-side protection `B- / P-` jest rozdzielane wewnatrz `battery_power`, a nie jeszcze jako pelny kontrakt miedzy wszystkimi modulami repo,
- to jest swiadome ograniczenie biezacej architektury, nie blad samego schematu modulu.

## BOM modulu
- `U_CHG`: `TP4056`, obudowa `HSOP-8-1EP`
- `R_PROG`: domyslnie `1.2k`
- `C_CHG_VCC`: domyslnie `10uF`
- `C_CHG_BAT`: domyslnie `1uF`
- `U_BMS`: `DW01A`, obudowa `SOT-23-6`
- `R_BMS_VDD`: `470`
- `C_BMS_VDD`: `0.1uF`
- `R_BMS_CS`: `2k`
- `Q_BMS`: `FS8205A`, obudowa `SOT-23-6`
- `R_PACK_POS`: `0R` link utrzymujacy logiczny podzial `BAT_RAW_VCC` / `VBAT_PROT`

Szczegoly BOM:
- [bom.md](/Users/michal/Desktop/pcb/docs/modules/battery_power/bom.md)

## Layout
- `TP4056` trzymaj blisko wejscia `V_USB` i z dobra referencja termiczna do `GND`
- `C_CHG_VCC` trzymaj przy `VCC/GND` `TP4056`
- `C_CHG_BAT` trzymaj przy `BAT/GND` `TP4056`
- `DW01A` i `FS8205A` trzymaj jako jeden klaster przy sciezce `B- / P-`
- szerokie sciezki prowadzenia pradu baterii sa wazniejsze niz sygnaly sterujace `OC/OD/CS`

Szczegoly placementu:
- [placement.md](/Users/michal/Desktop/pcb/docs/modules/battery_power/placement.md)

Checklista review:
- [review_checklist.md](/Users/michal/Desktop/pcb/docs/modules/battery_power/review_checklist.md)

## Powiazany kod
- `library/modules/battery_power.py`
- `library/modules/charger.py` jako warstwa kompatybilnosci
- `library/modules/power_path.py`
- `profiles/power_li_ion.md`

## Generator
- modul jest wpiety do `framework/module_registry.py`,
- generator traktuje go jako zrodlo `BAT_RAW_VCC` i `VBAT_PROT`,
- `power_path` korzysta z `VBAT_PROT`, a `battery_gauge` korzysta z `VBAT_PROT`.

## Weryfikacja
- `TP4056` ma teraz jawne podpiete `TEMP`, `PROG`, `GND`, `EP`, `VCC`, `BAT`, `CE`
- `DW01A` ma jawne podpiete `OD`, `OC`, `CS`, `VCC`, `GND`
- `FS8205A` ma jawne przypisanie `S1/D1/S2/G2/D2/G1`
- do dalszej walidacji pozostaje docelowy prad ladowania, termika `TP4056` i systemowa decyzja, po ktorej stronie ochrony ma siedziec fuel gauge
