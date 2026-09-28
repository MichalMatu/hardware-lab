# boards/

Ten katalog zawiera tylko aktywne warianty PCB rodziny Growclip.

Po cleanupie stare eksperymenty zostaly przeniesione do `../archive/boards/`. Nie ma jeszcze boardu oznaczonego jako produkcyjny.

Zasady aktywnego boardu sa opisane w `BOARD_CONTRACT.md`.

## Planowane pierwsze warianty

- `growclip_s3_basic` - prosty HAT do profilu S3,
- `growclip_c6_basic` - prosty HAT do profilu C6.

Aktualne profile-kandydaci w `../hosts/` bazuja na oficjalnych Espressif DevKitC-1, ale pozostaja `draft`, dopoki nie potwierdzimy zgodnosci z fizycznymi plytkami przeznaczonymi do Growclip.

Nie tworzymy boardu jako kopii hosta. Board wskazuje host profile i korzysta z jego `pinout.toml` oraz `mechanical.toml`.

## Zasada

Board powinien byc cienka kompozycja:
- host profile,
- zero lub kilka reusable modules,
- tylko board-specific polaczenia i layout.
