# ESP32-S3 devkit 2x22 host profile

Status: **draft / model do wyboru**.

Ten katalog rezerwuje profil dla pierwszego hosta Growclip S3 z dwoma headerami po 22 piny.

Przed utworzeniem `growclip_s3_basic` trzeba:
1. wskazac dokladny model devboardu,
2. zapisac oficjalne zrodlo pinoutu,
3. zmierzyc / potwierdzic geometrie headerow i obrys,
4. oznaczyc 5 V, 3.3 V, GND, EN/BOOT i strapping pins,
5. zapisac antenna keep-out,
6. dopiero wtedy utworzyc maszynowo czytelny `pinout.py` / `mechanical.py`.

Nie przenosimy placeholderow `Lxx/Rxx` ze starego eksperymentalnego boardu.
