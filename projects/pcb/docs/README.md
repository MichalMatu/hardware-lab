# PCB documentation

Aktywne `docs/` zawiera tylko wiedze potrzebna do biezacej, modularnej bazy Growclip.

## Struktura

- `components/<name>/README.md` - syntetyczne notatki o konkretnym ukladzie. To biblioteka wiedzy; obecność wpisu nie oznacza uzycia w boardzie.
- `modules/<name>/` - kontrakt i zrodla dla aktywnego reusable module z `library/modules/<name>.py`.

Dokumentacja eksperymentalnych zestawow bez aktywnej implementacji nie powinna pozostawac w `docs/modules/`. Surowe paczki producentow, duplikaty datasheetow i niezaklasyfikowane pliki nie sa przechowywane w aktywnym drzewie.

## Source of truth

Kolejnosc zaufania:

1. oficjalny datasheet / design guide,
2. oficjalny reference design producenta,
3. zweryfikowany `host.toml` / `pinout.toml` / `mechanical.toml`,
4. kontrakt modulu i jego kod,
5. board-specific dokumentacja.

Historyczne implementacje sa w `../archive/`. Usuniete materiały eksperymentalne mozna odzyskac z historii Gita, jesli znowu stana sie potrzebne.
