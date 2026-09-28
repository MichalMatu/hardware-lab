# archive/

Historyczne implementacje zachowane jako reference, ale wykluczone z aktywnego workflow.

- `boards/axp2101_devkitc38_reference/` - poprzedni HAT ESP32-DevKitC V4 + AXP2101.
- `boards/esp32_s3_hat_experiment/` - eksperymentalna przerobka starego HAT-a na 2x22.
- `legacy_tooling/` - poprzedni generator, skrypty i vendored SKiDL reference.
- `MIGRATION_STATUS.md` - historyczny status migracji.

`archive/` nie jest miejscem do dalszego rozwoju. Reusable element wraca swiadomie do `library/`, `hosts/` albo aktywnego boardu.

Nie archiwizujemy kazdego przypadkowego pliku. Duplikaty datasheetow, surowe vendor packi, niezaklasyfikowane materialy i stare docs bez aktywnej implementacji sa usuwane z biezacego drzewa; historia Gita pozostaje mechanizmem odzyskiwania.
