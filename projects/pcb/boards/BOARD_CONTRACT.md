# Growclip board contract

Aktywny board jest cienka kompozycja produktu. Nie jest kopia host profile ani generator calego systemu.

## Odpowiedzialnosc boardu

Board definiuje tylko:

- identyfikator i rewizje produktu,
- wskazanie jednego profilu hosta z `../../hosts/`,
- liste wymaganych reusable modules,
- polaczenia specyficzne dla tego wariantu,
- board-specific placement, outline/cutouts i routing,
- wyniki ERC/DRC/BOM dla konkretnej rewizji.

Board nie powinien definiowac ponownie:

- pinoutu devboardu,
- rozstawu headerow hosta,
- ograniczen strapping konkretnego MCU,
- wymiarow hosta,
- wspolnych implementacji I2C/RTC/battery/power itp.

## Minimalny manifest

Pierwszy manifest moze byc bardzo prosty:

```toml
[board]
id = "growclip_s3_basic"
revision = "A"
status = "prototype"

[host]
profile = "esp32_s3_devkit_44"

[features]
modules = []
```

Pusta lista modulow jest poprawna. Minimalny HAT moze na poczatku zawierac tylko host sockets, test points lub proste wyprowadzenia potrzebne do zweryfikowania mechaniki.

## Host gate

- `draft` host: dozwolony dla koncepcji i prototypu, ale wynik nie moze byc oznaczony jako production-ready.
- `verified` host: wymagany przed zatwierdzeniem Gerberow/PCBA do produkcji.
- board nie moze lokalnie nadpisywac pinoutu hosta tylko po to, aby ominac gate.

## Warstwy

Nie ma globalnego wymagania 4 warstw. Board wybiera 2 lub 4 warstwy zależnie od realnej elektroniki:

- prosty HAT bez szybkich/krytycznych blokow zasilania: preferuj 2 warstwy,
- zlozony PMIC, wymagajace power integrity/EMI lub routing: rozważ 4 warstwy.

## Workflow

1. Zweryfikuj lub wybierz host profile.
2. Utworz minimalny board manifest.
3. Dodaj tylko potrzebne moduly.
4. Wygeneruj/utworz schemat i placement bez autoroutingu.
5. ERC.
6. DRC i kontrola mechaniczna.
7. Dopiero potem BOM/Gerber/produkcja.
