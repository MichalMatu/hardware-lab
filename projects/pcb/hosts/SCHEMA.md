# Host profile contract

Host profile opisuje konkretny gotowy devboard, na ktory nakladany jest HAT Growclip. Nie opisuje funkcji produktu.

## Pliki profilu

Minimalny profil:

- `README.md` - status, zrodla i uwagi dla czlowieka,
- `host.toml` - maszynowo czytelna tozsamosc i tylko zweryfikowane dane,
- `pinout.py` - opcjonalnie, gdy pinout jest juz zweryfikowany,
- `mechanical.py` - opcjonalnie, gdy geometria jest juz zweryfikowana.

Brakujacych danych nie uzupelniamy placeholderami. Nieznana wartosc ma pozostac nieobecna.

## `host.toml`

Sekcja obowiazkowa:

```toml
[host]
id = "esp32_s3_example"
family = "esp32-s3"
status = "draft" # draft | verified
vendor = ""
model = ""
```

Pola `vendor` i `model` musza zostac uzupelnione przed zmiana statusu na `verified`.

Po weryfikacji mozna dodac:

```toml
[power]
logic_v = 3.3

[mechanical]
board_width_mm = 0.0
board_height_mm = 0.0

[[headers]]
name = "left"
pins = 0
pitch_mm = 2.54
x_mm = 0.0
y_mm = 0.0
```

Wartosci w przykladzie nie sa domyslnymi parametrami hosta.

## Pinout

Kazdy pin powinien miec co najmniej:
- fizyczny numer / pozycje na headerze,
- nazwe nadrukowana przez producenta,
- rzeczywisty GPIO lub rail,
- role specjalne: power, ground, EN, BOOT, strapping, USB/JTAG itp.,
- ograniczenia uzycia.

## Zasada weryfikacji

Status `verified` wymaga zrodla producenta albo pomiaru/inspekcji potwierdzonej na fizycznym module. Podobny devboard nie jest wystarczajacym zrodlem.
