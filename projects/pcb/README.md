# Growclip PCB Workspace

Elastyczna baza PCB dla rodziny Growclip. Pierwsze wersje sa prostymi HAT-ami / carrierami do gotowych devboardow ESP32-S3 i ESP32-C6. Nie ma jednego obowiazkowego MCU, PMIC-a ani zestawu funkcji.

## Stan bazowy

- aktywne boardy: **0**,
- profile hostow: **2 draft** (`ESP32-S3-DevKitC-1 v1.1`, `ESP32-C6-DevKitC-1 v1.2`),
- aktywne reusable modules: `i2c_bus`, `axp2101_pmic`,
- KiCad 10 workflow i wspolne helpery sa zweryfikowane lokalnie,
- stare boardy i legacy tooling sa odseparowane w `archive/`.

To jest celowy stan startowy. Nowy board powstaje dopiero po potwierdzeniu konkretnego fizycznego hosta.

## Architektura

- `hosts/` - pinout, mechanika i ograniczenia konkretnego devboardu.
- `library/` - kod wielokrotnego uzytku: kontrakty, walidatory, moduly SKiDL i neutralne helpery KiCada.
- `docs/components/` - male notatki o komponentach; wpis nie oznacza, ze komponent jest aktywnie uzywany.
- `docs/modules/` - dokumentacja tylko modulow, ktore maja aktywna implementacje w `library/modules/`.
- `boards/` - tylko aktywne warianty produktu; board ma byc cienka kompozycja hosta i modulow.
- `archive/` - historyczne implementacje i tooling, nie aktywna baza rozwoju.

## Zasady

1. **Host first.** Nie zgadujemy pinoutu ani geometrii podobnego devboardu.
2. **Minimal first.** Pierwszy wariant zawiera tylko funkcje potrzebne teraz.
3. **Reuse before copy.** Wspolna funkcja trafia do `library/`, nie do kolejnej kopii boardu.
4. **Board jest cienki.** Nie duplikuje pinoutu ani mechaniki hosta.
5. **Draft != production.** Produkcyjny board wymaga hosta `verified`.
6. **2 warstwy sa normalne.** 4 warstwy stosujemy tylko wtedy, gdy uzasadnia to elektronika.
7. **Brak autoroutera.** Krytyczny routing pozostaje kontrolowany w KiCadzie.

## Start pracy

Najpierw uruchom:

```bash
python3 projects/pcb/validate_workspace.py
```

Potem:

1. wybierz fizyczny devboard,
2. przejdz checklistę `hosts/PHYSICAL_VERIFICATION.md`,
3. uzupelnij i zatwierdz jego profil,
4. utworz minimalny `boards/growclip_<host>_<variant>/board.toml`,
5. dodaj tylko wymagane moduly,
6. wykonaj ERC, DRC i kontrole mechaniczna.

Szczegoly kontraktow: `hosts/PROFILE_CONTRACT.md`, `boards/BOARD_CONTRACT.md`, `requirements.md`.
