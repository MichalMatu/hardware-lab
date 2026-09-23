# Architektura

## Modul lokalnych bibliotek

Kod aplikacji jest dzielony na lokalne biblioteki PlatformIO:

- `AppCore`: typy, konfiguracja, logger i kontroler aplikacji;
- `AppCamera`: inicjalizacja OV2640, snapshot, LED flash;
- `AppStorage`: SD_MMC, zapis zdjec i logow;
- `AppNetwork`: Wi-Fi, portal konfiguracji, czas systemowy;
- `AppTelegram`: minimalny klient Telegram Bot API;
- `AppMotion`: detekcja ruchu i cooldown;
- `AppWeb`: lokalny panel administracyjny.

## Zasady pamieci

- Nie kopiowac JPEG do `String`, `std::vector` ani duzych buforow RAM.
- `camera_fb_t` musi byc zwrocony przez `esp_camera_fb_return()` w tej samej sciezce, ktora go pobrala.
- Bufory tekstowe maja miec stale rozmiary i jawne limity.
- JSON z Telegrama ma byc parsowany tylko w minimalnym zakresie potrzebnym do komend.
- Zdjecie jest wysylane i zapisywane strumieniowo z bufora kamery.

## Przeplyw zdarzen

1. `AppController` startuje konfiguracje, logger, kamere, SD, siec i panel.
2. `TelegramTask` pobiera komendy i przekazuje je do kontrolera.
3. `WebTask` obsluguje panel i akcje administracyjne.
4. `MotionTask` pilnuje cooldownu i zglasza zdarzenie ruchu.
5. Snapshot jest wykonywany pojedyncza sciezka, chroniona przed rownoleglym dostepem.

## Zakres MVP

MVP ma byc najpierw stabilny:

- konfiguracja przez portal;
- snapshot z panelu i Telegrama;
- wyslanie JPEG do Telegrama;
- zapis na SD, jesli karta dziala;
- podstawowy status i logi;
- prosty mechanizm motion/cooldown do dalszego strojenia na realnym obrazie.

Streaming 24/7, OTA i rozbudowany frontend nie sa celem pierwszej wersji.
