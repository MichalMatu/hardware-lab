# ESP32-C6 DevKitC host candidate

Status: **draft**.

Referencyjnym kandydatem dla pierwszego Growclip C6 jest oficjalny **Espressif ESP32-C6-DevKitC-1 v1.2** z dwoma headerami po 16 pinow.

`pinout.toml` zawiera uproszczone mapowanie fizycznych pinow J1/J3 na zasilanie/GPIO na podstawie oficjalnego user guide.

Profil pozostaje `draft`, dopoki nie potwierdzimy na fizycznym module Growclip:
- zgodnosci konkretnego modelu/revision,
- rozstawu headerow, obrysu i orientacji pin 1,
- antenna keep-out i kolizji obu portow USB-C,
- lokalnych ograniczen mechanicznych HAT-a.

Wazne ograniczenia:
- GPIO4, GPIO5, GPIO8, GPIO9 i GPIO15 sa pinami strapping,
- GPIO8 jest uzywany przez onboard RGB LED,
- GPIO12/GPIO13 tworza natywna pare USB D-/D+.

Nie zakladamy zgodnosci mechanicznej ani pinowej z profilem S3. Po lokalnej weryfikacji ten profil moze przejsc na `verified` i stac sie baza `growclip_c6_basic`.
