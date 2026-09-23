# Wymagania ogólne: PCB workspace

Ten dokument opisuje wspólne wymagania jakościowe, inżynieryjne i biznesowe dla płytek w tym repozytorium. Złotym standardem jest obecnie projekt `ESP32-DevKitC V4 HAT` oparty o `AXP2101`.

## 1. Cel dokumentu
Zdefiniować podstawowy standard jakości: strukturę katalogów, wymagania projektowe, praktyki layoutu oraz listę kontrolną gotowości produkcyjnej (commercialization).

## 2. Organizacja i Środowisko
Każda płytka jest osobnym folderem pod `boards/` (np. `boards/esp32_devkitc_hat/`).
Używamy **KiCad** (do ręcznego routingu i podglądu) oraz biblioteki **SKiDL** (do generowania schematów kodem Python).

Typowe ustawienie zmiennych środowiskowych:
- `KICAD_SYMBOL_DIR` - ścieżka do symboli KiCada.
- `KICAD_FOOTPRINT_DIR` - ścieżka do footprintów.

## 3. Minimalne Wymagania Funkcjonalne
Każdy projekt w repozytorium **musi** jasno definiować:
- Budżet prądowy i logikę zasilania (Power Path).
- Piny i interfejsy (I2C, SPI, UART).
- Wymagania mechaniczne (rozmiar, złącza THT, keep-out zone dla anten).
- Wygenerowany plik **BOM (.csv)** dopasowany pod usługę montażu (np. JLCPCB SMT).

## 4. Wymagania Technologiczne (Stackup)
- Dla zaawansowanych układów zasilających (PMIC impulsowy) **wymagany jest stackup 4-warstwowy**. Zapewnia on litą płaszczyznę masy (GND Plane) kluczową dla:
  - Odprowadzania ciepła (thermal pad z przelotkami do wylewki).
  - Zapobiegania emisji EMI (krótkie pętle powrotne prądu pod cewkami).
  - Ułatwienia routingu dla napięć dystrybucyjnych (5V, 3V3).

## 5. Dobre Praktyki Projektowe
- **Zasilanie (Decoupling):** Lokalne odsprzęganie blisko pinów układu. Dla PMIC kondensatory `VIN` oraz cewki przetwornic muszą leżeć maksymalnie blisko pinów wykonawczych układu (np. na tej samej warstwie, odsunięte o max 2-3 mm).
- **Zasada Rotacji Elementów:** Elementy (zwłaszcza dwupadowe) należy obracać w skrypcie layoutu tak, aby linie wirtualne (Ratsnest) nie przecinały się nawzajem. Upraszcza to manualny routing i eliminuje konieczność przelotek dla kluczowych sygnałów.
- **RF i Boot:** Brak ścieżek pod anteną układu bezprzewodowego. Piny strappingowe ESP32 (`IO0`, `IO2`, `IO12`, `IO15`) traktować jako "do not pull" przy starcie, by nie zepsuć bootloadera.

## 6. Lista Kontrolna Komercjalizacji (Dev Board Productization)
Zwykły działający obwód to nie produkt. Płytki przygotowywane do sprzedaży muszą bezwzględnie posiadać:
- [ ] **Otwory Montażowe:** Przynajmniej 2-4 otwory o standardowych średnicach (M2.5 lub M3) w narożnikach.
- [ ] **Złącze Baterii:** Wykorzystywać standard rynkowy `JST-PH 2.0mm` zamiast ogólnych footprintów pin-header.
- [ ] **Oznaczenia Silkscreen:** Czytelnie opisać polaryzację złącza baterii (`+`/`-`), oznaczyć piny wejścia/wyjścia na złączach i dodać logiczną nazwę/URL projektu.
- [ ] **Zabezpieczenie Polaryzacji (Opcjonalnie):** W przypadku wpinania baterii zaleca się implementację Reverse Polarity Protection na P-MOSFET, aby chronić wrażliwy układ ładowania PMIC-a przed pomyłką użytkownika.
- [ ] **Wyprowadzenie Interfejsów:** Jeżeli PMIC posiada wolne LDO, należy wystawić je na dostępne headery dla docelowego developera.

## 7. Wymagania Jakościowe i Weryfikacja
- Brak błędów ERC (Electrical Rules Check) na wygenerowanej netliście.
- Brak kolizji fizycznych (Courtyard Overlaps) - do walidacji przez skrypt Pythona zanim płytka trafi do KiCada.
- Elementy w BOM przyporządkowane do "Basic Parts" fabryki, gdy to możliwe, aby obniżyć koszty produkcji partii.
