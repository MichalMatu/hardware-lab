# Physical host verification

Ta checklista sluzy do potwierdzenia, ze konkretny devboard lezacy na biurku rzeczywiscie odpowiada profilowi w `hosts/`.

Nie wystarczy, ze plytka ma ten sam MCU albo tyle samo pinow.

## 1. Tozsamosc

- [ ] producent i model zgadzaja sie z `host.toml`,
- [ ] rewizja plytki jest znana lub zgodna z profilem,
- [ ] wariant modulu (np. WROOM-1 / WROOM-1U / WROOM-2) jest zgodny z zalozeniami,
- [ ] jesli producent stosuje batch/PW number, zapisano go gdy ma znaczenie dla rewizji.

## 2. Mechanika

Porownaj z `mechanical.toml`:

- [ ] szerokosc plytki,
- [ ] wysokosc plytki,
- [ ] liczba pinow w obu headerach,
- [ ] pitch 2.54 mm lub inny zadeklarowany pitch,
- [ ] rozstaw osi headerow,
- [ ] orientacja pin 1,
- [ ] polozenie USB,
- [ ] polozenie BOOT/RESET,
- [ ] antena i wymagany keep-out.

Dla krytycznych wymiarow preferowany jest pomiar suwmiarka lub porownanie z oficjalnym rysunkiem wymiarowym.

## 3. Pinout

Porownaj nadruk plytki i dokumentacje z `pinout.toml`:

- [ ] 5 V,
- [ ] 3V3,
- [ ] wszystkie GND,
- [ ] RST/EN,
- [ ] BOOT,
- [ ] GPIO uzywane przez planowany Growclip,
- [ ] native USB D+/D-, jesli ma byc uzywane,
- [ ] piny strapping,
- [ ] piny zajete przez RGB LED lub inne onboard peripherals.

## 4. Zmiana statusu

Dopiero po przejsciu checklisty ustaw w `host.toml`:

```toml
[verification]
physical_board_match = true
```

Status `verified` powinien byc nadany dopiero, gdy rownoczesnie zweryfikowano oficjalny pinout, oficjalna mechanike i konkretna fizyczna plytke.

Profil `draft` moze byc uzywany do prototypowania koncepcyjnego, ale validator nie pozwoli zatwierdzic na nim boardu `production`.
