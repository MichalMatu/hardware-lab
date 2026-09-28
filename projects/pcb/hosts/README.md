# hosts/

Profile gotowych modulow deweloperskich uzywanych jako host dla HAT-ow Growclip.

Host profile opisuje fizyczna i elektryczna kompatybilnosc, a nie funkcje produktu.

Kazdy profil powinien docelowo zawierac:
- dokladna nazwe i wariant plytki,
- zrodlo pinoutu,
- geometrie headerow,
- mapowanie pinow,
- zasilanie,
- EN/BOOT/strapping,
- antenna keep-out,
- ograniczenia mechaniczne.

Szczegolowy kontrakt i warunki przejscia z `draft` do `verified` opisuje `PROFILE_CONTRACT.md`.
Checklistę porownania profilu z konkretna fizyczna plytka opisuje `PHYSICAL_VERIFICATION.md`.

Profile oznaczone jako `draft` nie moga byc podstawa produkcyjnego PCB bez weryfikacji. Kod w `library/host_profile.py` udostepnia wspolna walidacje tej zasady.
