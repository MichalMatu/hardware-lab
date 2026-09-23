# ESP32-DevKitC AXP2101 HAT

Kompletny board referencyjny dla zasilacza (HAT-a) opartego na zaawansowanym układzie PMIC **AXP2101**, zaprojektowany pod płytki ewaluacyjne `ESP32-DevKitC V4` (i kompatybilne `38-pin`).

## Workflow i Struktura Projektu
1. **Ten projekt jest generowany pół-automatycznie**: Footprinty, przypisania pinów i dokładne położenie komponentów zarządzane są z poziomu skryptu w języku Python.
2. **Lokalny run**: Aby zbudować płytkę z kodu, użyj `./run_with_kicad_python.sh` w głównym katalogu.
3. Wrapper odpala najpierw `src/main.py` (budowanie netlisty z użyciem narzędzia SKiDL), a następnie `pcb/layout_automation.py`.
4. Skrypt z automatyzacją (layout_automation) resetuje trasy, konfiguruje 4-warstwowy "stackup" i precyzyjnie pozycjonuje wszystkie fizyczne komponenty zgodnie z ustaloną mapą (bez żadnych kolizji).
5. **Routing odbywa się całkowicie ręcznie** – zrezygnowaliśmy ze skryptów auto-routingu dla krytycznych ścieżek prądowych, aby pozostawić pełną swobodę poprawnego wylania miedzi (polygonów).
6. Całość dokumentacji deweloperskiej i biznesowej znajduje się w folderze `docs/`.

## Aktualny stan (Ready-to-Route)
- Płytka jest docięta idealnie pod wymiar rodziny `ESP32-DevKitC V4` (38-pin).
- Pinout hosta zdefiniowano jako jawne sieci `ESP_*`.
- Skomplikowany blok PMIC `AXP2101` został w 100% zintegrowany ze schematem i upakowany pozycjami fizycznymi pomiędzy obydwoma złączami THT na warstwie spodniej (Bottom).
- Optymalizacja orientacji elementów (cewek, tranzystorów, rezystorów) maksymalnie rozplątała wirtualne siatki połączeń (Ratsnest).
- Generacja do gotowych plików BOM dla usług montażu powierzchniowego (np. JLCPCB SMT).

## Sterowanie AXP2101 z ESP32
PMIC komunikuje się bezpośrednio z procesorem poprzez:
- `IO21/22` jako `I2C` (SDA/SCL)
- `IO32` jako `IRQ` (przerwania od zasilacza)
- `IO33` jako `PWROK` (Power-OK)
- `IO25` jako opcjonalny `CHGLED` (wskaźnik ładowania)
- `IO27` jako wirtualny `PWRON_BTN`

## Architektura Zasilania
- HAT nie posiada własnego złącza USB; portem wejściowym do ładowania układu jest pin `5V` ("VBUS") zaczerpnięty bezpośrednio z pinu USB ESP32.
- Logika wysterowująca zasila procesor napięciem `3.3V` płynącym z wyjścia przetwornicy głównej (DCDC1) z użyciem przełącznika `TPS22917L`:
  - Gdy podłączone jest USB (`ESP_5V` aktywne): Load-switch odciera HAT-a od pinu 3.3V procesora. W ten sposób ESP32 korzysta z własnego, fabrycznego stabilizatora z USB i zapobiegamy wstecznemu zasilaniu (back-feeding) do HAT-a.
  - Gdy USB jest odłączone: Switch automatycznie łączy główną przetwornicę DCDC1 AXP2101 na pin `ESP_3V3`, zasilając bezstratnie procesor z baterii.

## 4-Warstwowy Stackup (Kluczowy dla PMIC-a)
Płytka celowo korzysta z taniej technologii 4-warstwowej w celu odprowadzania prądów rzędu >2A z zachowaniem pełnej stabilności i rewelacyjnej dyssypacji ciepła:
1. `F.Cu` (Top): Gniazda dla procesora i krytyczne, zewnętrzne trasy sygnałowe.
2. `In1.Cu` (Power): Wewnętrzna dystrybucja zasilania dla `5V` i `3V3`.
3. `In2.Cu` (Solid GND): Lita płaszczyzna masy ułożona bezpośrednio obok pracujących tranzystorów PMIC, gwarantująca natychmiastową ucieczkę prądów powrotnych o wysokich częstotliwościach i ekranująca EMI.
4. `B.Cu` (Bottom): Kompletny blok AXP2101 z przyległymi kondensatorami, cewkami oraz ucieczkami (escape routing).

## Wskazówki Placementu dla GPIO ESP32
- Prawa, górna strefa (`IO21/22/23/19/18`): Idealne miejsce na złącza dla szybkich magistrali (`I2C` i `SPI`).
- Lewa, górna strefa (`VP/VN/34/35`): Piny Input-Only (np. pod przetwornik ADC dla dodatkowych pomiarów).
- Dolne piny należy traktować ostrożnie (tzw. Strapping Pins modyfikujące bootowanie): `IO0/2/12/15` oraz para `TX/RX` i sprzętowe wejścia złącza pamięci flash (`D0/D1/D2/D3/CMD/CLK`).

## Dokumenty Wsparcia i Komercjalizacja
W folderze `docs/` znajdziesz cenne dokumenty:
- `monetization_plan.md` – Strategia sprzedaży tego układu jako dev-boardu oraz gotowe oznaczenia numerów "Basic Components" pozwalające wyprodukować całe urządzenie usługą JLCPCB bez ponoszenia dodatkowych opłat "Extended Part" za szpule maszyn SMT.
- `spec.md` - wczesna specyfikacja.
- Do dyspozycji również znajduje się plik `out/jlcpcb_bom.csv`.
