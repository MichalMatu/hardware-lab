# boards/

Ten katalog zawiera tylko aktywne warianty PCB rodziny Growclip.

Po cleanupie stare eksperymenty zostaly przeniesione do `../archive/boards/`. Nie ma jeszcze boardu oznaczonego jako produkcyjny.

## Planowane pierwsze warianty

- `growclip_s3_basic` - prosty HAT do konkretnego, zweryfikowanego devboardu ESP32-S3.
- `growclip_c6_basic` - prosty HAT do konkretnego, zweryfikowanego devboardu ESP32-C6.

Nie tworzymy tych katalogow dopoki nie wybierzemy dokladnego hosta i nie zapiszemy jego mechaniki/pinoutu w `../hosts/`.

## Zasada

Board powinien byc cienka kompozycja:
- host profile,
- kilka reusable modules,
- tylko board-specific polaczenia i layout.
