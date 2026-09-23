# docs/components/

Notatki dla konkretnych ukladow, jesli datasheet nie wystarcza.

## Kiedy dodawac wpis

- gdy uklad ma kilka wariantow i trzeba zapisac wybrany wariant,
- gdy istotne sa piny, strapowanie, footprint albo szczegoly z app note,
- gdy chcesz zanotowac decyzje projektowe i kompromisy.

## Struktura

- `docs/components/<component_name>/README.md`

PDF-y i app notes powinny lezec przy odpowiednim module w `docs/modules/<module_name>/references/`.
Ten katalog sluzy do syntetycznych notatek roboczych, decyzji projektowych i pinoutow.

## Aktywne wpisy

- `docs/components/axp2101/`: PMIC z toru zasilania HAT-a.
