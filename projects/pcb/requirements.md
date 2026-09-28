# Wymagania ogolne: Growclip PCB workspace

Ten dokument definiuje zasady dla elastycznej rodziny plytek Growclip. Domyslny model rozwoju to prosty HAT / carrier do gotowego devboardu, a nie od razu kompletna plytka z wlasnym MCU i rozbudowanym PMIC.

## 1. Host najpierw

Kazdy board musi wskazywac konkretny profil hosta z `hosts/`.
Profil hosta powinien zawierac:
- dokladny model / wariant devboardu,
- liczbe i geometrie headerow,
- zweryfikowany pinout,
- dostepne 5 V / 3.3 V / GND,
- piny EN/BOOT/strapping oraz ograniczenia startowe,
- antenna keep-out i istotne ograniczenia mechaniczne.

Nie wolno uzupelniac brakujacego pinoutu na podstawie podobnego devboardu.

## 2. Minimalna kompozycja

Nowy board zaczynamy od najmniejszego zestawu funkcji potrzebnego dla danego wariantu Growclip.
Funkcje takie jak bateria, PMIC, RTC, boost, dodatkowe sensory lub wyjscia mocy sa opcjonalnymi modulami.

Nie ma globalnego wymagania uzywania AXP2101 ani konkretnego ukladu zasilania.

## 3. Wspolne moduly

Reusable logika powinna trafic do `library/`, jezeli:
- ma sens dla wiecej niz jednego boardu,
- ma jasno zdefiniowane wejscia/wyjscia,
- jest opisana w `docs/modules/`,
- nie zawiera zalozen mechanicznych konkretnego hosta.

Kod board-specific zostaje w `boards/<board>/`.

## 4. Technologia PCB

Domyslnie wybieramy najprostszy stackup, ktory spelnia wymagania elektryczne i mechaniczne.
- 2 warstwy sa akceptowalne dla prostych HAT-ow.
- 4 warstwy stosujemy wtedy, gdy uzasadnia to power integrity, EMI, gestosc routingu, USB/RF lub wymagania konkretnego ukladu.
- Zaawansowany PMIC impulsowy moze wymagac 4 warstw, ale nie jest to globalna regula dla calego workspace.

## 5. Zasilanie i interfejsy

Kazdy board musi jawnie opisac:
- skad bierze zasilanie,
- czy moze zasilac hosta i w jakich warunkach,
- zabezpieczenie przed back-feedem, jesli istnieje wiecej niz jedno zrodlo,
- budzet pradowy,
- uzywane I2C/SPI/UART/USB/GPIO,
- poziomy napiec i pull-up/pull-down.

## 6. Mechanika i RF

Przed routingiem nalezy zweryfikowac:
- rozstaw headerow i orientacje hosta,
- dostep do USB, BOOT, RESET i innych wymaganych elementow,
- brak kolizji z elementami na devboardzie,
- keep-out anteny,
- sensowne polozenie zlacz zewnetrznych,
- courtyard i edge clearance.

## 7. Weryfikacja

Przed uznaniem boardu za gotowy:
- ERC bez niewyjasnionych bledow,
- DRC bez niewyjasnionych bledow,
- brak niezamierzonych unconnected pads,
- zweryfikowany pinout hosta,
- zweryfikowane footprinty krytycznych elementow,
- przejrzany power path,
- BOM i oznaczenia produkcyjne dopiero wtedy, gdy wariant ma isc do produkcji.

## 8. Produkcja

JLCPCB/LCSC jest preferowanym workflow, ale optymalizacja pod Basic Parts nie moze wymuszac gorszego ukladu elektrycznego.
Silkscreen powinien jasno opisywac zasilanie, polaryzacje i istotne zlacza.

## 9. Zasada prostoty

Jesli funkcja nie jest potrzebna w pierwszej wersji boardu, nie dodajemy jej "na przyszlosc".
Elastycznosc ma wynikac ze wspolnych host profiles i reusable modules, a nie z jednego przeladowanego PCB.
