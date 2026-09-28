# library/kicad/

Male helpery wspolne dla automatyzacji PCB przez `pcbnew`.

Pierwszy zestaw zostal wyciagniety z legacy boardow, ale API zostalo uproszczone i pozbawione zalozen o AXP2101, liczbie pinow hosta lub konkretnej geometrii.

`core.py` udostepnia:
- `mm()` - konwersja mm do jednostek KiCad,
- `point_mm()` - punkt w mm,
- `find_pad()` - wyszukanie pada po numerze,
- `footprint_by_ref()` - wyszukanie footprintu po reference.

Ten modul wymaga interpretera, ktory widzi `pcbnew` z zainstalowanego KiCada. Nie powinien byc importowany przez narzedzia, ktore nie potrzebuja KiCada.

Nie przenosimy tu board-specific placementow, napisow ani stref funkcjonalnych.
