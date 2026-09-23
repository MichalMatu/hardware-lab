# ESP32-CAM Telegram Motion Camera

Projekt firmware dla modulu **ESP32-CAM z kamera OV2640** w wariancie zgodnym z AI-Thinker. Celem jest stabilna kamera-zdjecownik: Telegram, zapis na microSD, lokalny panel WWW, provisioning Wi-Fi i detekcja ruchu oparta o obraz.

## Status

- Sprzet zidentyfikowany ze zdjec jako ESP32-CAM / OV2640 z lampa LED na GPIO4.
- Widoczny port szeregowy: `/dev/cu.usbserial-110`.
- Projekt jest przygotowywany pod PlatformIO i Arduino framework.
- Biblioteki aplikacyjne sa lokalne w repozytorium, pod `lib/`.

## Najwazniejsze zalozenia

- Kamera ma byc traktowana jako zasob krytyczny: szybkie zwracanie framebufferow, brak kopiowania JPEG bez potrzeby.
- Domyslny profil zdjec: JPEG VGA/SVGA, z jakoscia dobrana pod stabilnosc i Telegram.
- microSD pracuje ostroznie, preferowany tryb 1-bit ze wzgledu na konflikt GPIO4 z LED flash.
- Konfiguracja trzymana w NVS przez `Preferences`.
- Panel WWW i portal konfiguracji korzystaja z wbudowanych bibliotek Arduino/ESP32.
- Token Telegrama i dane sieci nie sa wpisywane na stale do kodu.

## Dokumentacja

- [Cel i plan](docs/GOAL_AND_PLAN.md)
- [Sprzet](docs/HARDWARE.md)
- [Architektura](docs/ARCHITECTURE.md)

## Workflow

1. Otworz folder w VS Code.
2. Po utworzeniu lub zmianie `platformio.ini` przeladuj okno VS Code, zeby PlatformIO wczytalo projekt.
3. Buduj z CLI:

```sh
pio run
```

4. Upload przez wykryty port:

```sh
pio run -t upload --upload-port /dev/cu.usbserial-110
```

5. Monitor:

```sh
pio device monitor -p /dev/cu.usbserial-110 -b 115200
```
