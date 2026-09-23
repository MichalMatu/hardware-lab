# battery_power Placement

## Priorytet placementu
1. `TP4056` blisko wejscia `V_USB`
2. `C_CHG_VCC` i `C_CHG_BAT` bezposrednio przy pinach `TP4056`
3. `DW01A` i `FS8205A` blisko sciezki `B- / P-`
4. szeroka i czytelna sciezka dla pradu baterii przed porzadkowaniem sygnalow sterujacych

## Reguly praktyczne
- pad termiczny `TP4056` lacz do solidnego pola masy
- sciezka `V_USB -> U_CHG.VCC` nie powinna byc dluga ani waska
- sciezka `U_CHG.BAT -> BAT_RAW_VCC` powinna byc krotka i bez niepotrzebnych odnog
- `DW01A`, `R_BMS_VDD`, `C_BMS_VDD` trzymaj jako ciasny klaster
- `FS8205A` ustaw tak, by nie pomylic orientacji z pinoutem `S1/D1/S2/G2/D2/G1`
- prowadzenie `BAT_RAW_GND -> Q_BMS -> GND` powinno byc czytelne i szerokie

## Czego unikac
- odsuniecia kondensatorow `TP4056` o kilka centymetrow od ukladu
- traktowania `B-` i `P-` jako tego samego wezla w samym schemacie modulu
- prowadzenia waskich gardeł w glownej sciezce pradu baterii
- obracania `FS8205A` bez ponownego sprawdzenia numeracji pinow i kierunku sterowania `OC/OD`

## Uwaga termiczna
- `TP4056` przy wyzszych pradach ladowania szybko robi sie ograniczeniem termicznym
- na malej plytce lepiej zalozyc nizszy prad ladowania niz teoretyczne `1A`, jesli nie ma miejsca na sensowne odprowadzenie ciepla
