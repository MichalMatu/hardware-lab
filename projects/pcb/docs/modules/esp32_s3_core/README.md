# esp32_s3_core

Modul dokumentacji dla rdzenia opartego o ESP32-S3.

## Zakres
- zasilanie 3.3 V,
- reset, boot i strapping,
- USB, UART, Flash/PSRAM,
- wymagania RF i layoutowe dla wariantu z modulem lub antena.

## Glowny komponent
- `ESP32-S3`

## Powiazany kod
- `library/modules/mcu.py`
- `library/modules/usb.py`
- `library/modules/rf.py`
- `profiles/esp32-s3.md`

## Weryfikacja
- potwierdzic domeny zasilania i odsprzeganie,
- potwierdzic `CHIP_PU`, `GPIO0`, `GPIO3`, `GPIO45`, `GPIO46`,
- potwierdzic rezystory szeregowe USB i UART,
- potwierdzic wymagania RF, USB i kwarcu z guideline.
