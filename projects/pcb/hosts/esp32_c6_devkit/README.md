# ESP32-C6 devkit host profile

Status: **draft / model do wyboru**.

Ten katalog rezerwuje profil hosta dla pierwszego HAT-a Growclip C6.

Przed utworzeniem `growclip_c6_basic` trzeba wybrac konkretny devboard C6 i zweryfikowac:
- oficjalny pinout,
- geometrie headerow,
- zasilanie,
- EN/BOOT/strapping,
- piny zajete przez funkcje radiowe/peryferia,
- antenna keep-out i dostep do USB.

Nie zakladamy zgodnosci mechanicznej ani pinowej z profilem S3.
