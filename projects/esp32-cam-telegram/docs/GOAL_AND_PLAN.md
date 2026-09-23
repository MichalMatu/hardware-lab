# Cel i plan

## Cel

Stworzyc stabilny firmware dla ESP32-CAM OV2640, ktory:

- laczy sie z Wi-Fi i pozwala skonfigurowac dane przez lokalny portal;
- robi zdjecia na zadanie z panelu WWW i Telegrama;
- wykrywa ruch na podstawie obrazu i wysyla alarm ze zdjeciem;
- zapisuje zdjecia i logi na microSD, gdy karta jest dostepna;
- udostepnia prosty panel statusu w LAN;
- minimalizuje uzycie DRAM, kopiowanie buforow i ryzyko fragmentacji heap;
- daje sie budowac i utrzymywac w PlatformIO.

## Definicja gotowosci

Firmware uznajemy za gotowy, gdy:

- `pio run` przechodzi bez bledow;
- urzadzenie startuje na porcie szeregowym i raportuje heap/PSRAM/kamere;
- pierwsze uruchomienie wystawia portal konfiguracji;
- po konfiguracji dzialaja komendy `/status` i `/photo`;
- zdjecie jest wysylane do whitelisted chat ID w Telegramie;
- karta SD nie jest wymagana do pracy, ale jesli jest dostepna, zdjecia sa zapisywane;
- brak narastajacego spadku heap w dluzszym tescie robienia zdjec.

## Etapy

1. Fundament projektu
   - `platformio.ini`, lokalne biblioteki aplikacyjne, logger, typy i konfiguracja.
   - Potwierdzenie builda dla `board = esp32cam`.

2. Kamera
   - Inicjalizacja OV2640 z pinoutem AI-Thinker.
   - Snapshot JPEG bez kopiowania bufora.
   - Sterowanie LED flash na GPIO4 z uwzglednieniem konfliktu SD.

3. Konfiguracja i siec
   - NVS przez `Preferences`.
   - Portal AP do wpisania Wi-Fi, tokena Telegrama, chat ID i hasla panelu.
   - Synchronizacja czasu przed TLS.

4. Telegram
   - Minimalny lokalny klient HTTPS: `getUpdates`, `sendMessage`, `sendPhoto`.
   - Whitelist chat ID, timeouty, backoff i redakcja tokena w logach.

5. microSD i panel WWW
   - Mount `SD_MMC` w trybie 1-bit.
   - Katalogi `/photos/manual`, `/photos/motion`, `/logs`.
   - Panel statusu, reczny snapshot, ostatnie zdjecie, reset konfiguracji.

6. Detekcja ruchu
   - Najpierw stabilny tryb testowy z cooldownem.
   - Potem profil detekcyjny i porownywanie blokow/ROI.
   - Osobne progi: liczba blokow, suma roznic, cooldown.

7. Utwardzanie
   - Watchdog, reconnect Wi-Fi, odzyskiwanie po bledzie kamery.
   - Test dlugiego dzialania i obserwacja `free heap`, `min free heap`, PSRAM.

## Decyzje techniczne

- Bez zdalnych `lib_deps` dla logiki aplikacji; kod projektu jest lokalny w `lib/`.
- Brak duzych `String` w sciezkach krytycznych kamery i Telegrama.
- Framebuffer kamery jest zapisywany/wysylany jako strumien bajtow, bez kopiowania do osobnych buforow.
- Komendy i zdarzenia powinny przechodzic przez male, jawne struktury.
- Funkcje blokujace musza miec timeout.

## Ryzyka

- Slabe zasilanie 5 V moze wygladac jak bledy kamery, TLS albo losowe restarty.
- GPIO4 jest wspoldzielone logicznie przez LED flash i SD_MMC D1.
- TLS i wysylka zdjec przez HTTPS moga powodowac chwilowe piki pamieci.
- Przelaczanie profili kamery do detekcji ruchu wymaga testow na realnym module.
