# Host profile contract

Profil hosta opisuje konkretny gotowy devboard, na ktory nakladany jest HAT Growclip. Profil nie opisuje funkcji produktu.

## Pliki profilu

Docelowo profil hosta sklada sie z trzech niezaleznych warstw danych:

- `host.toml` - tozsamosc, rodzina, rewizja i stan weryfikacji,
- `pinout.toml` - fizyczne mapowanie pinow headerow,
- `mechanical.toml` - geometria boardu, headerow i wymagane keep-outy.

Board Growclip nie powinien kopiowac tych wartosci do swojego kodu. Powinien wskazywac profil hosta i korzystac z jego danych.

## Minimalne metadane

Kazdy `host.toml` musi zawierac:

```toml
[host]
id = "directory_name"
family = "esp32-s3" # albo esp32-c6
status = "draft"    # draft | verified | deprecated
vendor = ""
model = ""
revision = ""
```

`draft` oznacza, ze profil moze byc rozwijany i analizowany, ale nie wolno traktowac go jako zatwierdzonej podstawy PCB.

## Pinout

`pinout.toml` opisuje oba headery wedlug numeracji producenta. Minimalnie kazdy header musi miec komplet pinow w kolejnosci fizycznej. Nie stosujemy placeholderow typu `L10`/`R12` jako substytutu nieznanego GPIO.

## Mechanika

`mechanical.toml` trzyma dane niezalezne od konkretnego HAT-a, co najmniej:

```toml
[board]
width_mm = 0.0
height_mm = 0.0
antenna_edge = "top"
usb_edge = "bottom"

[headers]
count = 2
pins_per_header = 0
pitch_mm = 2.54
row_spacing_mm = 0.0
center_from_side_edge_mm = 0.0

[keepout]
antenna = "required"
usb = "required"
normalized_geometry = false
```

Wymiary z dokumentacji producenta moga byc zapisane juz dla profilu `draft`. `normalized_geometry = false` oznacza, ze znamy wymiary bazowe, ale nie mamy jeszcze kompletnego wielokata/strefy do automatycznego rysowania keep-outu.

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
- minimalne wymagane clearances dla HAT-a,
- zgodnosc danych z fizyczna plytka, ktora ma byc uzywana z Growclip.

Dane mechaniczne i pinout maja pochodzic z dokumentacji lub pomiaru konkretnej plytki. Nie kopiujemy mapowania z podobnego devboardu tylko dlatego, ze ma tyle samo pinow.

## Zasada dla boardow

Aktywny board Growclip moze wskazywac profil `draft` tylko podczas prac koncepcyjnych. Generowanie lub zatwierdzanie PCB do produkcji musi wymagac profilu `verified`.
