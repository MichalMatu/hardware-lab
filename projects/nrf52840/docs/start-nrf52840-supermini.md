# nRF52840 SuperMini / nice!nano - start z PlatformIO

## Aktualny status projektu

Projekt jest teraz standalone:

- `platformio.ini` w katalogu glownym jest glowna konfiguracja buildu.
- `boards/` zawiera lokalna definicje `nicenano`, wiec build nie wymaga
  globalnej instalacji boarda w PlatformIO.
- `src/main.cpp` jest domyslnym firmware.
- `vendor/` jest tylko lokalnym katalogiem research i jest ignorowany przez
  git.

Podstawowy build:

```sh
pio run
```

Sprawdzony wynik standalone:

```text
supermini SUCCESS
RAM:   3.5% (used 8748 bytes from 248832 bytes)
Flash: 6.5% (used 53180 bytes from 815104 bytes)
```

## Lokalny research/vendor

Do `vendor/` zostaly pobrane plytkie kopie projektow, ktore sa najbardziej
przydatne do szybkiego startu z nRF52840 SuperMini / nice!nano. Ten katalog
nie jest czescia repozytorium i nie jest potrzebny do budowania firmware:

| Katalog | Repo | Commit | Rola |
| --- | --- | --- | --- |
| `vendor/nicenano-example` | `https://github.com/bertrik/nicenano-example` | `a2a8000` z 2025-12-07 | Najlepszy minimalny szablon PlatformIO. Ma gotowy `supermini/platformio.ini`, lokalne `boards/nicenano.json` i wariant `boards/nicenano/variant.*`. |
| `vendor/nicenano-nrf52-supermini-platformio-support` | `https://github.com/ICantMakeThings/Nicenano-NRF52-Supermini-PlatformIO-Support` | `ea662c5` z 2026-03-25 | Definicja plytki plus gotowe przyklady PlatformIO: Blinky, BLE Beacon, HID, odczyt baterii, SSD1306. |
| `vendor/nRFMicro-Arduino-Core` | `https://github.com/pdcook/nRFMicro-Arduino-Core` | `a83161e` z 2024-10-11 | Arduino core dopasowany do nRFMicro, SuperMini nRF52840 i nice!nano. Ma warianty `SuperMini_nRF52840`, `nice_nano` oraz przyklady Bluefruit. |
| `vendor/Adafruit_nRF52_Arduino` | `https://github.com/adafruit/Adafruit_nRF52_Arduino` | `343ab5f` z 2026-05-12 | Glowny punkt odniesienia dla Arduino/Bluefruit na nRF52. Bardzo duzo przykladow BLE, USB i hardware. |
| `vendor/zephyr` | `https://github.com/zephyrproject-rtos/zephyr` | `7d1bca7cf` z 2026-06-21 | Profesjonalne sample BLE i RTOS. Dobre jako repo referencyjne, ale ciezsze niz Arduino/PlatformIO. |

Nie pobieralem na tym etapie `btlejack`, `gattacker`, `btstack` ani
`bluekitchen/raccoon`, bo nie sa najlepszym pierwszym krokiem do uruchomienia
tej plytki w PlatformIO. Warto do nich wrocic dopiero po opanowaniu uploadu,
BLE UART, beacon/HID i podstaw pinoutu.

## Rekomendowana kolejnosc

1. Zacznij od `vendor/nicenano-example/supermini`.
   To najczystszy szablon do pierwszego builda i uploadu.

2. Przejrzyj `vendor/nicenano-nrf52-supermini-platformio-support/Platformio Example code`.
   Tam sa gotowe projekty:
   - `Blinky`
   - `BLE-BEACON`
   - `HID Example`
   - `Read Batt voltage`
   - `Read Batt voltage | SSD1306 display`

   Uwaga: te przyklady nie sa samodzielne po samym sklonowaniu repo. Ich
   `platformio.ini` uzywa `board = nicenano`, ale definicja plytki jest w
   katalogu repo `.platformio/`, nie w samym przykladzie. Lokalny build
   `Blinky` zakonczyl sie bledem:

   ```text
   UnknownBoard: Unknown board ID 'nicenano'
   ```

   Najprosciej uzywac tych katalogow jako zrodla kodu `main.cpp` i przenosic
   go do szablonu `vendor/nicenano-example/supermini`, ktory juz ma lokalne
   `boards/nicenano.json`, `boards/nicenano/variant.*` oraz
   `board_build.variants_dir = boards`.

   W katalogu glownym workspace jest teraz dodany `platformio.ini`, ktory
   rozwiazuje ten problem dla pierwszego przykladu `Blinky`. Z katalogu
   glownego dziala:

   ```sh
   pio run
   ```

   Domyslny env `support_blinky` uzywa:

   ```ini
   boards_dir = vendor/nicenano-example/supermini/boards
   board_build.variants_dir = vendor/nicenano-example/supermini/boards
   ```

   Lokalny build z katalogu glownego przeszedl poprawnie:

   ```text
   support_blinky SUCCESS
   RAM:   1.2% (used 3084 bytes from 248832 bytes)
   Flash: 2.6% (used 21028 bytes from 815104 bytes)
   ```

3. Do BLE Arduino uzywaj przykladow z:
   `vendor/Adafruit_nRF52_Arduino/libraries/Bluefruit52Lib/examples`

   Najbardziej praktyczne katalogi na start:
   - `Peripheral/bleuart`
   - `Peripheral/beacon`
   - `Peripheral/blehid_keyboard`
   - `Peripheral/blehid_mouse`
   - `Peripheral/throughput`
   - `Central/central_scan`
   - `Central/central_bleuart`
   - `DualRoles/dual_bleuart`
   - `Hardware/adc`
   - `Hardware/adc_vbat`

4. Gdy Arduino/PlatformIO dziala stabilnie, przejdz do Zephyra:
   `vendor/zephyr/samples/bluetooth`.

   Warte obejrzenia sample:
   - `beacon`
   - `central`
   - `observer`
   - `peripheral`
   - `peripheral_hids`
   - `peripheral_nus`
   - `hci_usb`
   - `hci_uart`
   - `mesh`

## Pierwszy build PlatformIO

Sprawdzony lokalnie projekt:

```sh
cd vendor/nicenano-example/supermini
pio run
```

Wynik lokalnej kompilacji:

```text
SUCCESS
RAM:   3.5% (used 8764 bytes from 248832 bytes)
Flash: 7.2% (used 58664 bytes from 815104 bytes)
```

PlatformIO uzylo:

```text
PlatformIO Core 6.1.19
platform: nordicnrf52@10.11.0
board: nice!nano
framework: arduino
framework-arduinoadafruitnrf52: 1.10700.0
```

## Upload na plytke

Aktualnie `pio device list` pokazuje `/dev/cu.usbserial-120` z VID:PID
`1A86:7523`, ale ten port nalezy do ESP32, nie do nRF52840. Nie uzywaj go do
uploadu na nRF.

Po double-reset nRF52840 zostal wykryty jako:

```text
/dev/cu.usbmodem1101
USB VID:PID=239A:00B3
Description: nice!nano
/Volumes/NICENANO
```

Upload sprawdzonego rootowego env `support_blinky` przeszedl poprawnie:

```sh
pio run -e support_blinky -t upload --upload-port /dev/cu.usbmodem1101
```

Wynik:

```text
Uploading .pio/build/support_blinky/firmware.zip
Device programmed.
SUCCESS
```

Po aktywowaniu firmware wolumen `NICENANO` znika, bo plytka wychodzi z
bootloadera. Ten konkretny `Blinky` nie musi pokazywac portu USB w normalnym
trybie. Do kolejnego uploadu wejdz znowu w bootloader przez double-reset
`RST` do `GND`.

## Gdy nRF nie pojawia sie na USB

Poczatkowy test systemowy nie wykryl nRF52840. Przez okolo minute przed
wejsciem w bootloader system widzial tylko ESP32 na `/dev/cu.usbserial-120`
oraz brak nowego wolumenu UF2 w `/Volumes`.

Kolejnosc diagnostyki:

1. Odlacz ESP32 albo przynajmniej ignoruj `/dev/cu.usbserial-120`.
2. Uzyj kabla USB z danymi. Kable tylko do ladowania sa najczestsza przyczyna
   braku urzadzenia.
3. Podlacz nRF bezposrednio do Maca albo przez inny hub/adapter.
4. Zrob double reset: dwa szybkie nacisniecia `RESET`. Jesli nie ma przycisku,
   zewrzyj `RST` z `GND` dwa razy krotko.
5. Sprawdz, czy pojawil sie nowy port:

   ```sh
   pio device list
   ```

6. Sprawdz, czy pojawil sie dysk UF2:

   ```sh
   ls /Volumes
   ```

7. Na macOS dla natywnego USB nRF52840 zwykle nie trzeba sterownika. Sterowniki
   maja znaczenie dla zewnetrznych mostkow USB-UART, np. CH340/CP210x, ale
   nRF52840 SuperMini/nice!nano powinien pokazac sie przez natywne USB, jesli
   ma dzialajacy bootloader i kabel z liniami danych.

Jesli po tych krokach nadal nic sie nie pojawia, mozliwe scenariusze to: zly
kabel, problem z portem/hubem, brak bootloadera na plytce albo plytka w stanie,
ktory wymaga programatora SWD/J-Link/CMSIS-DAP do wgrania bootloadera.

Najpierw wejdz do projektu:

```sh
cd vendor/nicenano-example/supermini
```

Build:

```sh
pio run
```

Upload:

```sh
pio run -t upload
```

Monitor portu szeregowego:

```sh
pio device monitor
```

Jesli upload nie ruszy za pierwszym razem, nacisnij reset/double reset na
plytce i powtorz upload. README projektu PlatformIO Support wspomina, ze na
tych tanich klonach czasem trzeba uruchomic upload drugi raz.

## Minimalny `platformio.ini`

Szablon z `vendor/nicenano-example/supermini/platformio.ini`:

```ini
[platformio]
default_envs = supermini
src_dir = .

[env:supermini]
platform = nordicnrf52
board = nicenano
board_build.variants_dir = boards
framework = arduino
lib_deps =
    https://github.com/bertrik/minishell
```

Dla przykladow BLE z Bluefruit zwykle potrzebna jest zaleznosc Adafruit:

```ini
lib_deps =
    https://github.com/adafruit/Adafruit_nRF52_Arduino
```

Dla przykladow USB HID z PlatformIO Support uzywany jest:

```ini
board_build.arduino.adafruit_usb = tinyusb
```

## Pinout i nazwy pinow

W repo PlatformIO Support opis pinow jest w `README.md` i w grafice
`ICMTPINOUTNRF52840.svg`.

W przykladach tego repo numery z nadruku typu `102` odpowiadaja fizycznym
pinom Nordic `P1.02`. W kodzie uzywaj nazw w stylu:

```cpp
digitalWrite(PIN_102, HIGH);
```

Z README PlatformIO Support:

```text
SPI:  P1.11 SCK, P1.13 MOSI, P1.15 MISO
I2C:  P1.06 SCL, P1.04 SDA
UART: P0.06 TX,  P0.08 RX
```

Warianty i mapowanie pinow sa w:

```text
vendor/nicenano-example/supermini/boards/nicenano/variant.h
vendor/nicenano-example/supermini/boards/nicenano/variant.cpp
vendor/nRFMicro-Arduino-Core/variants/SuperMini_nRF52840/variant.h
vendor/nRFMicro-Arduino-Core/variants/nice_nano/variant.h
```

## Gdy chcesz uruchamiac przyklady PlatformIO Support bez przenoszenia kodu

Sa dwie rozsadne drogi:

1. Skopiowac lokalny katalog `boards/` ze sprawdzonego szablonu do wybranego
   przykladu i dodac do jego `platformio.ini`:

   ```ini
   board_build.variants_dir = boards
   ```

2. Zainstalowac definicje z repo PlatformIO Support globalnie do PlatformIO,
   zgodnie z jego README. Pliki zrodlowe definicji sa tutaj:

   ```text
   vendor/nicenano-nrf52-supermini-platformio-support/.platformio/platforms/nordicnrf52/boards/nicenano.json
   vendor/nicenano-nrf52-supermini-platformio-support/.platformio/packages/framework-arduinoadafruitnrf52/variants/nicenano/variant.h
   vendor/nicenano-nrf52-supermini-platformio-support/.platformio/packages/framework-arduinoadafruitnrf52/variants/nicenano/variant.cpp
   ```

Pierwsza droga jest lepsza do pracy w tym workspace, bo nie zmienia globalnej
instalacji PlatformIO.

## Co dalej

Najrozsadniejszy nastepny krok to utworzyc w katalogu glownym osobny projekt
roboczy, skopiowany z `vendor/nicenano-example/supermini`, zamiast edytowac
bezposrednio vendor. Wtedy `vendor/` zostaje referencja, a wlasny kod jest
oddzielony od pobranych projektow.
