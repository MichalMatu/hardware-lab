# battery_gauge Review Checklist

Checklist do koncowego review schematu, layoutu i uruchomienia modulu `battery_gauge`.

## 1. Schematic Sign-Off
- `U_GAUGE` to rzeczywiscie `MAX17048`, nie `MAX17049`
- pin `VDD` jest polaczony do `VBAT_PROT` / `PACK+`
- pin `CELL` pozostaje `NC` dla `MAX17048`
- pin `CTG` jest polaczony do `GND`
- pin `GND` jest polaczony do `GND`
- exposed pad `EP` jest polaczony do `GND`
- pin `QSTRT` jest polaczony do `GND`, jesli hardware quick-start nie jest wymagany
- pin `ALRT` jest podlaczony tylko wtedy, gdy system rzeczywiscie go uzywa
- `ALRT` ma pull-up do raila logiki hosta, jesli linia jest uzywana
- `SDA` i `SCL` sa podpiete do wspolnej magistrali I2C
- pull-upy `SDA/SCL` sa obecne tylko raz w systemie
- `C_GAUGE` = `0.1uF` i jest wpiety miedzy `VDD` i `GND`

## 2. Layout Sign-Off
- `U_GAUGE` siedzi blisko wejscia `VBAT_PROT`
- polaczenie `VBAT_PROT -> VDD` jest krotkie i bez zbednych odnog
- `C_GAUGE` jest maksymalnie blisko pinow `VDD` i `GND`
- `EP`, `GND` i `CTG` maja krotkie, bezposrednie wejscie w pole masy
- powrot masy dla `C_GAUGE` nie idzie przez waskie gardlo
- nie ma agresywnego przecinania referencji `GND` pod ukladem
- `SDA`, `SCL` i `ALRT` nie wymuszaja niepotrzebnego wydluzania sciezki `VBAT_PROT`
- footprint ukladu zgadza sie z obudowa `TDFN-8-1EP 2x2 mm`
- footprint kondensatora i rezystorow zgadza sie z zalozonym assembly flow

## 3. System Integration
- logika hosta jest zgodna poziomami z `ALRT`, `SDA` i `SCL`
- rail logiki pull-upow jest stabilny podczas pracy fuel gauge
- `VBAT_PROT` jest pobierane z punktu reprezentujacego system-side `PACK+`
- jesli w systemie jest protection FET / charger / power-path, gauge siedzi po stronie chronionej zgodnie z architektura produktu
- firmware zna adres I2C `MAX17048` i obsluguje brak odpowiedzi przy pustej baterii / odpieciu
- firmware ustawia lub swiadomie ignoruje alerty `ALRT`

## 4. Bring-Up
- odczyt rejestru wersji / ID po I2C dziala stabilnie
- odczyt napiecia ogniwa zgadza sie z pomiarem multimetrem w akceptowalnym zakresie
- SoC zmienia sie logicznie przy ladowaniu i rozladowaniu
- `ALRT` reaguje zgodnie z ustawionymi progami, jesli jest uzywany
- nie wystepuje losowe znikanie urzadzenia z magistrali I2C

## 5. Open Items Before Production
- potwierdzony konkretny producent i MPN dla `MAX17048`
- potwierdzony konkretny dielektryk i napiecie pracy dla `C_GAUGE`
- potwierdzona wartosc pull-up `ALRT` w kontekscie wybranego raila logiki
- potwierdzone strojenie `RCOMP` / modelu baterii w firmware, jesli wymagane przez dokladnosc produktu
- wykonany finalny review DFM / assembly dla obudowy `TDFN 2x2 mm`
