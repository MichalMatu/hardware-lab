# Growclip PCB Workspace

Ten katalog jest wspolna baza sprzetowa dla rodziny Growclip. Pierwsze wersje PCB maja byc prostymi HAT-ami / carrierami do gotowych modulow deweloperskich ESP32-S3 i ESP32-C6. Nie zakladamy jednego MCU, jednego ukladu zasilania ani jednego zestawu funkcji.

## Zasady architektury

1. **Host jest osobnym profilem.** `hosts/` opisuje konkretny devboard: mechanike, headery, pinout, zasilanie, piny strapping i keep-out anteny.
2. **Funkcje sa wspolnymi klockami.** `library/` zawiera reusable interfejsy i moduly, np. I2C lub AXP2101. Modul nie jest obowiazkowy tylko dlatego, ze istnieje.
3. **Board jest cienka kompozycja.** `boards/` zawiera tylko aktywne warianty Growclip i ich board-specific polaczenia, mechanike oraz layout.
4. **Najpierw minimalna wersja.** Zlozone zasilanie, bateria, RTC, dodatkowe sensory i inne funkcje dodajemy dopiero w wariancie, ktory ich potrzebuje.
5. **Wiedza jest wspolna.** Datasheety, reference designy, profile i checklisty w `docs/` oraz `profiles/` sa niezalezne od konkretnego boardu.

## Struktura

- `boards/` - aktywne plytki Growclip. Po cleanupie nie ma jeszcze boardu uznanego za produkcyjny.
- `hosts/` - profile fizycznych devboardow S3/C6.
- `library/` - wspolne interfejsy i implementacje modulow SKiDL.
- `docs/` - datasheety, reference designy, bring-up i dokumentacja modulow.
- `profiles/` - ogolne profile rodzin ukladow i domen funkcjonalnych.
- `archive/` - stare boardy i legacy tooling zachowane jako referencja, nie jako aktywna baza.
- `requirements.md` - aktualne zasady projektowe.
- `SKILL.md` - workflow pracy nad tym workspace.

## Aktualny kierunek

Pierwsze aktywne warianty powinny zaczac od:
- `growclip_s3_basic` - minimalny HAT do wybranego i zweryfikowanego devboardu ESP32-S3,
- `growclip_c6_basic` - minimalny HAT do wybranego i zweryfikowanego devboardu ESP32-C6.

Dokladnych pinoutow nie nalezy zgadywac. Najpierw wybieramy konkretny model devboardu i tworzymy jego profil w `hosts/`, dopiero potem skladamy board.

AXP2101 pozostaje wartosciowym reusable modulem i reference designem, ale nie jest juz centrum architektury Growclip.
