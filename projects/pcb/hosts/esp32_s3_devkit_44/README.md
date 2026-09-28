# ESP32-S3 DevKitC host candidate

Status: **draft**.

Referencyjnym kandydatem dla pierwszego Growclip S3 jest oficjalny **Espressif ESP32-S3-DevKitC-1 v1.1** z dwoma headerami po 22 piny.

`pinout.toml` zawiera uproszczone mapowanie fizycznych pinow J1/J3 na zasilanie/GPIO na podstawie oficjalnego user guide.

Profil pozostaje `draft`, dopoki nie potwierdzimy na fizycznym module Growclip:
1. zgodnosci konkretnego modelu/revision,
2. rozstawu headerow, obrysu i orientacji pin 1,
3. antenna keep-out i kolizji USB,
4. wariantu modulu WROOM/flash/PSRAM.

Wazne ograniczenia:
- rewizja v1.1 uzywa GPIO38 dla RGB LED; poczatkowa rewizja uzywala GPIO48,
- GPIO35/36/37 nie sa dostepne na czesci wariantow z octal flash/PSRAM,
- nie przenosimy placeholderow `Lxx/Rxx` ze starego eksperymentalnego boardu.

Dopiero po lokalnej weryfikacji mechaniki profil moze przejsc na `verified` i stac sie baza `growclip_s3_basic`.
