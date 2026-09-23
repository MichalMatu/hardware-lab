# Profil: ESP32-S3

Ten profil opisuje wspolne wymagania i dobre praktyki dla plytek opartych o ESP32-S3. Jest celowo ogolny - szczegoly i wartosci nalezy brac z dokumentacji konkretnego wariantu/modulu.

Zrodla:
- `docs/modules/esp32_s3_core/references/esp-hardware-design-guidelines-en-master-esp32s3.pdf` (Release master, 2026-03-03).
- `docs/modules/esp32_s3_core/references/esp32-s3_datasheet_en.pdf` (v2.2).

## 1. Wybor wariantu i integracja
- Zdecyduj, czy uzywasz modulu czy golego ukladu.
- Zdefiniuj typ anteny (PCB, zewnetrzna, u.FL) i wymagania RF.
- Zweryfikuj wymagane interfejsy: UART, USB-OTG, USB Serial/JTAG, I2C, SPI, GPIO.

## 2. Schematic checklist (najwazniejsze)
Power supply: pojedyncze zasilanie 3.3 V, wydajnosc pradowa minimum 0.5 A; dioda ESD i co najmniej 10 uF na wejsciu; 0.1 uF blisko VDD3P3_CPU i VDD3P3_RTC; 0.1 uF + 1 uF blisko VDD_SPI i bez zbyt duzych kondensatorow na VDD_SPI; VDD_SPI ma wewnetrzny LDO okolo 40 mA dla 1.8 V, a przy 3.3 V jest zasilane przez rezystor ~14 ohm z VDD3P3_RTC, wiec VDD3P3_RTC powinno pozostac powyzej 3.0 V; VDD3P3/VDDA wymagaja lokalnej pojemnosci (typowo 10 uF) i filtrow LC, a wszystkie domeny zasilania powinny startowac razem.
> [!WARNING]
> **Stabilność LDO:** Starsze regulatory (np. AMS1117) wymagają kondensatorów wyjściowych o wyższym ESR (np. tantalowych). Użycie samej ceramiki (MLCC) może prowadzić do oscylacji. Dla kondensatorów ceramicznych stosuj nowoczesne układy typu "MLCC stable" (np. AZ1117C).

Reset/CHIP_PU: CHIP_PU nie moze byc pozostawiony w stanie plywajacym; zalecane RC przy CHIP_PU to R=10 k oraz C=1 uF; utrzymuj sciezke CHIP_PU krotka; minimalne czasy stabilizacji i resetu to 50 us.

Strapping pins: GPIO0, GPIO3, GPIO45, GPIO46; zalecany pull-up na GPIO0; nie dodawaj duzych kondensatorow na GPIO0, bo moze to wymusic tryb download; GPIO45 kontroluje VDD_SPI, GPIO46 kontroluje boot i ROM print, GPIO3 wybiera zrodlo JTAG; stany strapping sa probkowane po resecie i pozostaja do wylaczenia zasilania; minimalny czas hold strapping to 3 ms po CHIP_PU.

Flash/PSRAM: preferuj modele flash/PSRAM zweryfikowane przez Espressif; dodaj footprinty rezystorow 0 ohm w szereg na liniach SPI dla strojenia; przy VDD_SPI jako zasilaniu upewnij sie, ze napiecie jest zgodne z wybranym flash/PSRAM.

Clock: obowiazkowy kwarc 40 MHz (dokladnosc okolo +/-10 ppm); zalecany element szeregowy na XTAL_P (startowo 24 nH) dla ograniczenia harmonicznych; opcjonalny kwarc 32.768 kHz wymaga ESR <= 70 k i zwykle nie wymaga rezystora biasujacego.

UART/SPI: zalecany rezystor szeregowy 499 ohm na U0TXD w celu redukcji harmonicznych; przy SPI dodaj rezystor szeregowy lub koralik ferrytowy oraz kondensator do masy na SPI_CLK (i opcjonalnie na innych liniach) blisko ukladu.

USB: pozostaw miejsce na elementy RC blisko ukladu; planuj ochrone ESD na liniach zewnetrznych.

## 3. PCB layout checklist (najwazniejsze)
Power/ground: szerokie sciezki zasilania (glowne >= 25 mil, VDD3P3 przy pinach 2/3 >= 20 mil, pozostale >= 10 mil); pojemnosc 10 uF na wejsciu zasilania i przy VDD3P3/VDDA; rozgalezianie w gwiazde po odsprzeganiu; pady masy ukladu lacz bezposrednio z poligonem masy; pod pad termiczny dodaj co najmniej 9 vias do masy.

Crystal: zachowaj izolacje masy i brak tras pod kwarcem; bez przelotek na liniach XTAL; dystans od pinu zegara co najmniej 2.0 mm; gesty stiching vias masy wokol; nie prowadz szybkich sygnalow pod kwarcem.

RF: sciezka RF 50 ohm, bez rozgalezien i bez przelotek, mozliwie krotka; CLC matching z elementami 0201 blisko pinu RF; stub dla kondensatora po stronie ukladu (ok. 15 mil dlugosci, ~100 ohm); pelna plaszczyzna masy pod RF i brak tras pod sciezka RF; keep-out pod antena we wszystkich warstwach; przy module na base board umiesc antene poza plytka bazowa lub zachowaj min. 15 mm clearance bez miedzi i elementow.

USB: prowadzenie jako para roznicowa 90 ohm +/-10%, rowna dlugosc; minimalizuj przelotki, a jesli sa konieczne dodaj pary powrotnych vias masy; ciagla warstwa referencyjna (masa) pod USB; otocz sciezki USB miedzia masy.

SPI/SDIO: dla SPI i SDIO utrzymuj kontrolowana impedancje i symmetryczne dlugosci (SDIO data w zakresie +/-50 mil od CLK), unikaj zmian warstw i zapewnij ciagla referencje masy; przy octal SPI zachowaj rowne dlugosci; dodaj odsprzeganie przy VDD_SPI oraz przy flash/PSRAM.

UART: rezystor szeregowy na U0TXD blisko ukladu i z dala od kwarcu; sciezki U0TXD/U0RXD mozliwie krotkie; otocz je masami i vias.

## 4. Minimalna checklista (ESP32-S3)
- [ ] Wybrany wariant modulu/ukladu i typ anteny.
- [ ] Zasilanie 3.3 V z zapasem pradowym i odsprzeganiem zgodnym z wytycznymi.
- [ ] CHIP_PU, strapping i boot zgodne z wytycznymi.
- [ ] Flash/PSRAM zgodne z VDD_SPI i z opcja strojenia (0 ohm).
- [ ] Kwarc 40 MHz poprawnie dobrany i polozony.
- [ ] RF, USB i szybkie interfejsy zgodne z zasadami layoutu.
- [ ] Testpointy dla zasilania i podstawowych interfejsow.

## 5. Thermal & Power Delivery
- **Zasilanie (LDO vs DCDC):** Z uwagi na chwilowe piki prądu dochodzące do 500 mA, dla projektów pracujących z zasilania powyżej 5 V zalecane są przetwornice Buck (np. SY8120) zamiast LDO, aby zredukować dyssypację ciepła.
- **Thermal Pad:** Obowiązkowe min. 9 przelotek (vias) o średnicy 0.3 mm pod padem termicznym EP, aby odprowadzić ciepło na ciągłą warstwę masy.
