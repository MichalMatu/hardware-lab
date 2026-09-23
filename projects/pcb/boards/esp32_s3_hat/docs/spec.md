# Specyfikacja: esp32_devkitc_hat

## 1. Cel i zakres
- Bazowy HAT pod `ESP32-DevKitC V4` i kompatybilne plytki `38-pin`.
- `rev A` zaczyna od poprawnej mechaniki host headerow i od integracji bazowego bloku `AXP2101`.
- Na tym etapie board ma juz subsystem zasilania, ale nie ma jeszcze wszystkich docelowych peryferiow HAT-a.

## 2. Zasilanie
- Wejscie bazowe: `ESP_5V` z host boarda.
- Masa wspolna: `GND`.
- Dodatkowo dostepne `ESP_3V3` z host boarda.
- HAT nie ma wlasnego portu `USB`; zasilanie i programowanie pozostaja po stronie host boarda.
- Osobne zasilanie zewnetrzne nie jest na tym etapie przewidziane.
- Dla wariantu z `AXP2101` net roboczy `V_USB` ma byc zasilany z `ESP_5V`, a nie z osobnego zlacza `USB`.
- Wejscie baterii jest juz przewidziane przez `J_BAT -> BAT_RAW_VCC`.
- Glowna lokalna szyna z `AXP2101` to `HAT_3V3`.
- `HAT_3V3 -> ESP_3V3` jest przelaczane przez dedykowany `load switch` `TPS22917L`.
- pin `ON` przelacznika jest sterowany obecnoscia `ESP_5V` z host boarda:
  - bez `ESP_5V` host moze byc zasilony z `HAT_3V3`
  - przy obecnym `ESP_5V` tor `HAT_3V3 -> ESP_3V3` ma zostac odciety z blokada reverse current
- Nie przewidujemy wariantu boost `5V` z baterii dla `rev A`.

## 3. MCU/SoC i peryferia
- MCU nie znajduje sie na tym boardzie; hostem jest zewnetrzny `ESP32-DevKitC`-class board.
- Krytyczne sygnaly bazowe dla przyszlych modulow HAT-a:
  - `ESP_GPIO21_SDA`
  - `ESP_GPIO22_SCL`
  - `ESP_GPIO23_MOSI`
  - `ESP_GPIO19_MISO`
  - `ESP_GPIO18_SCK`
  - pomocnicze `ESP_GPIO25/26/27/32/33`
- Piny strapowe i flash-memory sa zachowane w pinoucie, ale nie sa preferowane do dalszych funkcji.
- Dla wariantu HAT z `AXP2101` przyjmujemy domyslne mapowanie pomocnicze:
  - `ESP_GPIO32 -> AXP_IRQ`
  - `ESP_GPIO33 -> AXP_PWROK`
  - `ESP_GPIO25 -> AXP_CHGLED`
  - `ESP_GPIO27 -> AXP_PWRON_BTN`
  - `ESP_GPIO26` pozostaje zapasem

## 4. Interfejsy zewnętrzne
- `J_HOST_L`: lewy socket `1x19`, zgodny z `DevKitC V4 J2`
- `J_HOST_R`: prawy socket `1x19`, zgodny z `DevKitC V4 J3`
- Rozstaw rzedow: `25.40 mm`
- Raster pinow: `2.54 mm`
- `J_BAT`: wejscie baterii `BAT_RAW_VCC`, `GND`
- testpady serwisowe przy dolnej krawedzi:
  - `TP_GND`
  - `TP_VSYS`
  - `TP_VRTC`
  - `TP_VREF`
  - `TP_VMID`
  - `TP_TS`
  - `TP_PWRON`
- automatyczny tor `HAT_3V3 -> ESP_3V3`:
  - `U_HOST_3V3_SW`
  - `R_HOST_3V3_OFF`
  - `R_HOST_3V3_ON`

## 5. Programowanie i serwis
- Programowanie: przez `USB` host boarda i jego `USB-UART`.
- HAT nie dodaje osobnego toru programowania.
- Serwis obejmuje juz takze walidacje bazowych szyn `AXP2101`, przycisku `PWRON` i sygnalow statusowych.

## 6. Mechanika
- Mechanika referencyjna host boarda:
  - dlugosc `48.26 mm`
  - szerokosc `27.94 mm`
  - rozstaw rzedow headerow `25.40 mm`
- Na PCB HAT-a zaznaczony jest zarys host boarda i strefa anteny na warstwach opisowych.
- Obrys HAT-a ma byc dokladnie zgodny z `ESP32-DevKitC V4`, bez dodatkowej dolnej sekcji.
- Wszystkie elementy subsystemu `AXP2101` maja miescic sie w korytarzu pomiedzy `J_HOST_L` i `J_HOST_R`.

## 6a. Preferowane strefy funkcjonalne
- Caly blok `AXP2101` jest pakowany w srodkowym korytarzu pomiedzy dwoma rzedami headerow.
- Gorna czesc tego korytarza pozostaje bardziej ostrozna ze wzgledu na antene host boarda.
- Dolna czesc korytarza jest preferowana dla testpadow serwisowych, przycisku `PWRON`, `BAT_TS` i baterii, o ile wszystko nadal miesci sie w obrysie hosta.
- Prawa gorna strona hosta jest preferowana dla interfejsow cyfrowych:
  - `ESP_GPIO21_SDA`
  - `ESP_GPIO22_SCL`
  - `ESP_GPIO23_MOSI`
  - `ESP_GPIO19_MISO`
  - `ESP_GPIO18_SCK`
- Lewa gorna strona hosta jest preferowana dla wejsc analogowych:
  - `ESP_GPIO36_VP`
  - `ESP_GPIO39_VN`
  - `ESP_GPIO34`
  - `ESP_GPIO35`
- Lewa srodkowa strefa (`ESP_GPIO25/26/27/32/33`) pozostaje najbardziej elastyczna dla funkcji pomocniczych.
- Dolne rejony pinow sa mniej korzystne pod stale funkcje HAT-a:
  - `ESP_GPIO0_BOOT`, `ESP_GPIO2`, `ESP_GPIO12`, `ESP_GPIO15` sa zwiazane z boot strap
  - `ESP_UART0_TX`, `ESP_UART0_RX` sa wspoldzielone z programowaniem i logami
  - `ESP_FLASH_D0/D1/D2/D3/CMD/CLK` sa zarezerwowane dla flash
- Dla migracji starego breakouta `AXP2101` na HAT przyjmujemy kompaktowy uklad:
  - srodek: `U_PMIC`, cewki i krytyczne kondensatory
  - dolny odcinek korytarza: zlacza serwisowe, `PWRON`, LED i pomocnicze pasywy

## 7. RF
- Host board ma antene przy gornej krawedzi.
- Nad strefa anteny nie nalezy prowadzic gestej miedzi ani ustawiac glownych elementow HAT-a.
- Aktualny szkic layoutu zaznacza te strefe jako referencyjny keep-out mechaniczny.

## 8. Wymagania technologiczne PCB
- Docelowo `4` warstwy. Kolejnosc zgodna z `AXP2101 Design Guide V1.0 §5.2`: warstwa `GND` ma sasiadowac z warstwa, na ktorej siedzi PMIC i cewki DCDC (tu: `B.Cu`), by ekranowac przelaczanie DCDC od `VREF`.
  - `F.Cu`: sockety host, zlacze baterii, testpady, krytyczne sciezki top
  - `In1.Cu`: dystrybucja `5V/3V3`
  - `In2.Cu`: pelna masa `GND` (sasiednia z `B.Cu`)
  - `B.Cu`: `U_PMIC`, cewki, kondensatory dekupplujace PMIC, sygnaly bottom
- Trace widths dla high-current paths (DG §5.1): `VBUS`, `VMID`, `BAT`, `VSYS`, kazdy `VIN*` i `LX*`, `ALDOIN`, `BLDOIN`, `DC1SW`, `DC4SW` -> `> 150 mil (3.81 mm)`.
- Routing nie jest jeszcze wykonany; board ma byc punktem startowym pod dalszy layout.

## 9. Podział na moduły
- Host interface:
  - `J_HOST_L`
  - `J_HOST_R`
- PMIC core:
  - `U_PMIC`
  - `L_PMIC_SW`
  - `L_DCDC1`
  - krytyczne kondensatory `VBUS/BAT/VSYS/VRTC/VREF`
- Service / bring-up:
  - `J_BAT`
  - `TP_GND`
  - `TP_VSYS`
  - `TP_VRTC`
  - `TP_VREF`
  - `TP_VMID`
  - `TP_TS`
  - `TP_PWRON`
  - `U_HOST_3V3_SW`
  - `R_HOST_3V3_OFF`
  - `R_HOST_3V3_ON`
  - `SW_PWRON`
- Mechanical reference:
  - host board outline
  - antenna keep-out marker
- Future HAT circuitry: do dopiecia po ustaleniu funkcji plytki.

## 10. Konwencje nazewnictwa sieci
- Host-facing nets maja prefiks `ESP_`.
- Zasilanie:
  - `ESP_5V`
  - `ESP_3V3`
- UART:
  - `ESP_UART0_TX`
  - `ESP_UART0_RX`
- I2C:
  - `ESP_GPIO21_SDA`
  - `ESP_GPIO22_SCL`
- SPI:
  - `ESP_GPIO23_MOSI`
  - `ESP_GPIO19_MISO`
  - `ESP_GPIO18_SCK`
- Flash-reserved:
  - `ESP_FLASH_D0`
  - `ESP_FLASH_D1`
  - `ESP_FLASH_D2`
  - `ESP_FLASH_D3`
  - `ESP_FLASH_CMD`
  - `ESP_FLASH_CLK`

## 11. Weryfikacja i testy
- Kryteria biezacego etapu:
  - obrys HAT-a jest zgodny z `ESP32-DevKitC V4`
  - sockety `1x19` sa ustawione we wlasciwym rozstawie
  - pinout odpowiada `ESP32-DevKitC V4`
  - bazowy blok `AXP2101` jest juz obecny na schemacie i PCB
  - wszystkie elementy `AXP2101` sa umieszczone pomiedzy rzedami headerow
  - routing i strefy sa jeszcze w trakcie dopinania
