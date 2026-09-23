# PCB Workspace

Główne repozytorium (monorepo) przechowujące zautomatyzowane projekty płytek PCB oparte na Pythonie (SKiDL + API KiCad).

Aktualnie aktywnym i rozwijanym produktem jest **HAT pod `ESP32-DevKitC V4`** oparty o zaawansowany PMIC **AXP2101** w katalogu `boards/esp32_devkitc_hat/`.

Najważniejsze zasady jakościowe, organizacyjne i biznesowe opisane są w pliku `requirements.md`.

## Struktura repozytorium

- `boards/esp32_devkitc_hat/`: Główna, aktywna płytka HAT-a. Kompletny workflow opisany jest w jej lokalnym `README.md`.
- `library/interfaces.py`: Kontrakty pomiędzy fizycznymi modułami.
- `library/modules/`: Współdzielone bloki sprzętowe (np. `axp2101_pmic`, `i2c_bus`), projektowane tak, by można je było reużywać w innych płytkach.
- `docs/`: Centralna dokumentacja, PDF-y (datasheety), kontrakty modułów oraz strategie komercjalizacji (np. `monetization_plan.md`).
- `framework/`: Prototypowy generator deklaratywny (przygotowany pod przyszłe, mniej skomplikowane płytki oparte o `board.toml`).
- `requirements.md`: Twarde wymagania techniczne i biznesowe dla wszystkich projektów w repozytorium.
- `SKILL.md`: Konfiguracja i workflow pracy inżynieryjnej (przydatne jako persona dla asystentów AI).

## Architektura i Podejście

1. **Schematy generowane kodem (SKiDL):** Nie rysujemy schematów ręcznie. Cała logika połączeń (netlisty) wynika z kodu w Pythonie. Ułatwia to wersjonowanie (Git), ponowne użycie bloków (jak AXP2101) i audyty bezpieczeństwa.
2. **Precyzyjny Layout Automatyczny:** Footprinty i ich współrzędne aplikowane są skryptowo (`layout_automation.py`), co zapobiega powstawaniu kolizji mechanicznych (courtyards) i gwarantuje, że przy każdej przebudowie układ pozostaje "czysty".
3. **Ręczny Routing Krytyczny:** Ścieżki zasilania (DCDC) i wylewki miedzi (poligony) prowadzone są ostatecznie ręcznie. Skrypty auto-routingu zostały usunięte z produkcyjnego workflowu, ponieważ ręczne rozlanie litel masy (GND) na 4-warstwowej płytce ma kluczowe znaczenie dla stabilności PMIC.
4. **Gotowość Produkcyjna (JLCPCB):** Wymuszamy stosowanie standardowych komponentów ("Basic parts" w LCSC) z wygenerowanym `jlcpcb_bom.csv`, aby minimalizować koszty produkcji PCBA.
