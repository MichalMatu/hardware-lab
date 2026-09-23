# Sprzet

## Zidentyfikowany modul

Na podstawie zdjec w folderze:

- modul: ESP32-CAM w stylu AI-Thinker;
- SoC/modul radiowy: ESP32-S;
- kamera: OV2640;
- lampa LED: GPIO4;
- port szeregowy widoczny w systemie: `/dev/cu.usbserial-110`.

## Pinout kamery AI-Thinker

| Sygnal | GPIO |
|---|---:|
| PWDN | 32 |
| RESET | -1 |
| XCLK | 0 |
| SIOD | 26 |
| SIOC | 27 |
| Y9 | 35 |
| Y8 | 34 |
| Y7 | 39 |
| Y6 | 36 |
| Y5 | 21 |
| Y4 | 19 |
| Y3 | 18 |
| Y2 | 5 |
| VSYNC | 25 |
| HREF | 23 |
| PCLK | 22 |
| FLASH LED | 4 |

## Uwagi praktyczne

- Do uploadu ESP32-CAM zwykle wymaga trybu bootloadera: GPIO0 do GND podczas resetu, chyba ze adapter robi to automatycznie.
- Zasilanie powinno byc stabilne 5 V. Objawy slabego zasilania to m.in. restarty, bledy `esp_camera_init`, timeouty TLS i losowe zaniki Wi-Fi.
- Dla SD_MMC preferowany jest tryb 1-bit, poniewaz GPIO4 jest tez lampa LED.
- PSRAM powinna byc uzywana do framebufferow JPEG, ale logika aplikacji ma nadal oszczedzac DRAM.
