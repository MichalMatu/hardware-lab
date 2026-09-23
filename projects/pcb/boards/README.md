# boards/

Ten katalog przechowuje konkretne plytki. Aktualnie w repo jest tylko jedna aktywna plytka:

- `esp32_devkitc_hat/`: HAT pod `ESP32-DevKitC V4` (38-pin), hand-authored.

## Status frameworku

- `boards/_template/` zostal usuniety razem z innymi plytkami w cleanupie.
- `scripts/create_board.py` nadal istnieje, ale nie jest operacyjny bez `_template/`.
- `scripts/render_board.py` dziala dla manifestow z modulami wspieranymi w `framework/module_registry.py` (`axp2101_pmic`, `i2c_bus`). Aktualny HAT jest hand-authored i nie korzysta z renderera.

## Co trzymac lokalnie w plytce

- `board.toml`: sklad i wyjscia (informacyjnie; HAT nie jest renderowany).
- `spec.md`: wymagania konkretnej plytki.
- `pcb/`: pliki KiCad i automatyka layoutu.
- `config/`: lokalna konfiguracja SKiDL/KiCad.
- `src/main.py`: hand-authored kod SKiDL.

## Co trzymac centralnie

- `library/interfaces.py`: kontrakty miedzy modulami.
- `library/modules/`: wspolne moduly SKiDL (`axp2101_pmic`, `i2c_bus`).
- `docs/modules/`: dokumentacja i checklista kazdego modulu.
- `docs/components/`: notatki dla konkretnych ukladow.
