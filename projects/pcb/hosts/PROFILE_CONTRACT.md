# Host profile contract

Profil hosta opisuje konkretny gotowy devboard, na ktory nakladany jest HAT Growclip. Profil nie opisuje funkcji produktu.

## Minimalne metadane

Kazdy `host.toml` musi zawierac:

```toml
[host]
id = "directory_name"
family = "esp32-s3" # albo esp32-c6
status = "draft"    # draft | verified | deprecated
vendor = ""
model = ""
```

`draft` oznacza, ze profil moze byc rozwijany i analizowany, ale nie wolno traktowac go jako zatwierdzonej podstawy PCB.

## Warunki przejscia do `verified`

Przed zmiana statusu na `verified` profil musi miec zweryfikowane:

- dokladny vendor i model/revision devboardu,
- oficjalne zrodlo pinoutu lub dokumentacje producenta,
- liczbe pinow i geometrie obu headerow,
- rozstaw headerow i pitch,
- orientacje pin 1,
- piny 5 V / 3V3 / GND,
- EN/RESET i BOOT,
- piny strapping i piny z ograniczeniami startowymi,
- antenna keep-out oraz pozycje anteny,
- polozenie USB i innych elementow tworzacych kolizje mechaniczne,
- mapowanie fizyczny pin -> GPIO/funkcja,
- minimalne wymagane clearances dla HAT-a.

Dane mechaniczne i pinout maja pochodzic z dokumentacji lub pomiaru konkretnej plytki. Nie kopiujemy mapowania z podobnego devboardu tylko dlatego, ze ma tyle samo pinow.

## Zasada dla boardow

Aktywny board Growclip moze wskazywac profil `draft` tylko podczas prac koncepcyjnych. Generowanie lub zatwierdzanie PCB do produkcji musi wymagac profilu `verified`.
