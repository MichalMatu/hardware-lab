# battery_gauge Placement

## Priorytet placementu
1. `U_GAUGE` blisko wejscia `VBAT_PROT` / punktu `PACK+`
2. `C_GAUGE` bezposrednio przy pinach `VDD` i `GND`
3. `R_GAUGE_ALRT` moze byc blizej hosta lub linii wyjsciowej `ALRT`

## Reguly praktyczne
- trzymaj polaczenie `VBAT_PROT / PACK+ -> VDD` krotkie i bez zbednych odnog
- `GND` ukladu i exposed pad podlacz bezposrednio do pelnej referencji masy
- nie prowadz sygnalow pod ukladem, jesli utrudni to czysty powrot do `GND`
- `SDA`, `SCL` i `ALRT` nie sa szybkie; ich routing jest drugorzedny wzgledem czystego zasilania i masy
- jesli to breakout lub maly modul, trzymaj `U_GAUGE` i `C_GAUGE` jako jeden ciasny klaster

## Czego unikac
- oddalenia `C_GAUGE` od ukladu o kilka centymetrow lub przez waskie gardla
- prowadzenia `VBAT_PROT` dluga petla przez pol plytki przed `VDD`
- mieszania `ALRT` z pull-upami `SDA/SCL` w jednym miejscu, jesli utrudnia to czytelnosc modulu

## Uwaga termiczna
- `MAX17048` nie jest ukladem mocy; exposed pad pelni glownie role referencji `GND` i stabilnosci montazu
- w prostym 2-warstwowym module zwykle wystarczy solidne pole masy bez rozbudowanych thermal vias
