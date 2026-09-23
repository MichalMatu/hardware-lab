# esp32_hat

Notatka projektowa dla HAT-a pod rodzine ESP32.

## Cel

Zrobic HAT, ktory bedzie mechanicznie i elektrycznie kompatybilny z mozliwie duza liczba popularnych plytek dev board z ESP32, ale bez udawania, ze jedna geometria natywnie obsluzy wszystkie ekosystemy jednoczesnie.

## Wniosek projektowy

Nie ma jednego "uniwersalnego" footprintu HAT-a dla calego rynku ESP32.
Sa co najmniej cztery rozne rodziny mechaniczne:

- `ESP32-DevKitC / 38-pin DevKit V1 / NodeMCU-32S` - klasyczna rodzina dual-inline `2 x 19`
- `ESP32-DevKitM-1` - inna liczba pinow i inna geometria
- `Arduino Nano ESP32` - ekosystem `Nano`
- `Adafruit Feather ESP32-S3` - ekosystem `Feather / Wing`

Dlatego dla `rev A` wybieramy jeden glowny target natywny i pozostale rodziny traktujemy jako temat na adapter albo osobna rewizje.

## Glowny target natywny

### Rodzina `ESP32-DevKitC V4` / kompatybilne `38-pin` clone-family

To jest najlepszy punkt startu dla HAT-a "najbardziej kompatybilnego", bo laczy:

- oficjalny board referencyjny Espressif `ESP32-DevKitC V4`
- bardzo szeroko spotykana rodzine klonow sprzedawanych jako `ESP32 DevKit V1`, `NodeMCU-32S`, `ESP-WROOM-32 Dev Board`
- wygodny format `2 x 19` na pinach `2.54 mm`
- pelny breakout klasycznych sygnalow `I2C`, `SPI`, `UART`, ADC, DAC

### Parametry mechaniczne, ktore przyjmujemy dla `rev A`

- dwa rzedy gniazd `1 x 19`
- raster pinow `2.54 mm`
- dlugosc plytki referencyjnej `48.26 mm`
- szerokosc plytki referencyjnej `27.94 mm`
- rysunek wymiarowy `ESP32-DevKitC V4` pokazuje rozstaw rzedow headerow `25.40 mm`

W praktyce `rev A` HAT-a ma byc projektowany pod osadzenie na tej geometrii.
Kompatybilnosc z popularnymi klonami `38-pin` traktujemy jako cel, ale i tak trzeba bedzie zweryfikowac pinout konkretnych modeli przed zamowieniem.

## Rodziny nienatywne dla `rev A`

### `ESP32-DevKitM-1`

- oficjalny board Espressif
- ma `32` wyprowadzenia, a nie `38`
- nie powinien byc traktowany jako zgodny mechanicznie z HAT-em pod `DevKitC`

### `ESP32-PICO-KIT`

- oficjalny mini board Espressif
- to inna geometria i inna liczba pad/header positions
- nie jest dobrym targetem dla pierwszego, szeroko kompatybilnego HAT-a

### `Arduino Nano ESP32`

- nalezy do ekosystemu `Nano`
- jest atrakcyjny rynkowo, ale to osobna rodzina mechaniczna
- jesli bedziemy chcieli go wspierac, to przez adapter albo druga wersje PCB

### `Adafruit Feather ESP32-S3`

- nalezy do ekosystemu `Feather / FeatherWing`
- ma wlasny ustalony standard wymiarow i polozenia pinow
- nie probujemy go wspierac natywnie tym samym footprintem co `DevKitC`

### `LOLIN D32`

- popularna plytka, ale ma inna dlugosc i inny uklad ekosystemowy niz `DevKitC`
- traktowac jako kandydat do osobnej wersji lub adaptera, nie jako glowny target `rev A`

## Kontrakt elektryczny HAT-a dla szerokiej kompatybilnosci

### Co moze byc wymagane przez HAT

- `3V3`
- `GND`
- `SDA = GPIO21`
- `SCL = GPIO22`
- opcjonalnie `SPI`: `MOSI=23`, `MISO=19`, `SCK=18`
- opcjonalnie jeden bezpieczny `CS`, najlepiej konfigurowalny zworka / lutowanym mostkiem
- opcjonalnie pomocnicze GPIO z grupy `25`, `26`, `27`, `32`, `33`

### Czego nie robic jako twardego wymagania `rev A`

- nie wymagac `GPIO16/17` jako krytycznych, bo na czesci wariantow `WROVER` sa zarezerwowane
- nie opierac HAT-a o piny `D0`, `D1`, `D2`, `D3`, `CMD`, `CLK`, bo sa powiazane z flash/SPI memory
- nie zakladac, ze piny strapowe sa "wolne": `GPIO0`, `GPIO2`, `GPIO5`, `GPIO12`, `GPIO15` tylko ostroznie
- nie traktowac `GPIO34`, `GPIO35`, `GPIO36`, `GPIO39` jako zwyklych wyjsc; to sa piny wejsciowe / analogowe

## USB i zasilanie

### Decyzja dla `rev A`

- HAT nie dostaje wlasnego portu `USB`
- programowanie i debug pozostaja po stronie host boarda `ESP32-DevKitC / 38-pin`
- HAT bierze zasilanie z `5V` host boarda oraz wspolnej masy `GND`

### Wazne doprecyzowanie

Dla klasycznego `ESP32` z rodziny `DevKitC / NodeMCU-32S` nie korzystamy z "natywnego USB z ESP32", bo ten SoC nie wystawia natywnego interfejsu USB tak jak nowsze rodziny `S2/S3/C3`.
W praktyce korzystamy z portu `USB` host boarda i z jego lokalnego mostka `USB-UART`, a na HAT wyprowadzamy tylko zasilanie i sygnaly GPIO.

### Konsekwencje projektowe

- HAT powinien miec pin `5V` jako wejscie z host boarda
- HAT powinien miec lokalne `3V3` tylko wtedy, gdy naprawde potrzebujemy wlasnej przetwornicy albo separacji szumowej
- jezeli HAT ma w przyszlosci opcje zasilania zewnetrznego, trzeba rozwiazac to tak, aby nie backfeedowac `5V` do portu `USB` host boarda
- dla `rev A` najbezpieczniej zalozyc pojedyncze zrodlo zasilania: `USB` wpiete do dev boarda, a HAT tylko korzysta z `5V` i ewentualnie `3V3`

## Zalecenie dla `rev A`

Jesli HAT ma byc naprawde szeroko kompatybilny, to `rev A` powinien byc:

- projektowany mechanicznie pod `ESP32-DevKitC V4 / 38-pin`
- elektrycznie `I2C-first`
- z opcjonalnym `SPI` i kilkoma konfigurowalnymi GPIO
- bez twardego uzaleznienia od pelnego, konkretnego "clone pinoutu"

To da nam najwieksza szanse, ze jedna plytka bedzie pasowala do oficjalnego `DevKitC` i do duzej czesci popularnych klonow `38-pin`, a pozostale rodziny (`Nano`, `Feather`, `DevKitM`) bedziemy rozwiazywac adapterem albo osobna rewizja.

## Zrodla

- lokalna dokumentacja: `docs/reference_designs/esp32_hat/references/esp-dev-kits-en-master-esp32.pdf`
- Espressif `ESP32-DevKitC V4` user guide: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html
- Espressif `ESP32-DevKitC V4` dimensions PDF: https://dl.espressif.com/dl/schematics/esp32_devkitc_v4_dimensions.pdf
- Espressif `ESP32-PICO-KIT` user guide: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-pico-kit/user_guide.html
- WEMOS `D32`: https://www.wemos.cc/en/latest/d32/d32.html
- Arduino `Nano ESP32`: https://docs.arduino.cc/hardware/nano-esp32
- Adafruit Feather specification: https://learn.adafruit.com/adafruit-feather/feather-specification
