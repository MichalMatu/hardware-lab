# Hardware Lab

Hardware experiments, firmware starters, board bring-up projects and setup notes.

This repository consolidates several previously standalone repositories while retaining their Git history. Each project remains isolated in its own directory under `projects/`.

## Projects

- `projects/esp_rs` — ESP Rust experiments and starter firmware
- `projects/rp_pi2_zero` — Raspberry Pi Zero 2 W setup and experiments
- `projects/modules` — ESP32-family module experiments
- `projects/nrf52840` — nRF52840 SuperMini / nice!nano experiments
- `projects/LilyGo_4.7` — LilyGo T5 4.7-inch e-paper firmware
- `projects/milkV256mb` — Milk-V Duo 256 setup and experiments
- `projects/LILYGO_T_HIGH_GROW` — LILYGO T-HIGrow PlantStatus firmware and web UI (sanitized snapshot)
- `projects/esp32-cam-telegram` — ESP32-CAM/OV2640 Telegram motion camera (unfinished; build validated)
- `projects/esp32_s2_wifi` — ESP32-S2 USB NCM ↔ Wi-Fi bridge; working main snapshot plus preserved OLED variant
- `projects/pcb` — SKiDL/KiCad PCB workspace and board experiments (unfinished; hardware validation pending)

-  — retired ESP32-C6 Zigbee/low-level I/O extension prototype; maintained code absorbed into esp32s3_LiteGraph

## History and licenses

Most imported projects retain their original commits as merge ancestry. Some former standalone repositories have since been removed after verification. Projects with contaminated secret history may be imported as sanitized snapshots instead. Existing per-project license files and notices remain authoritative for their respective project directories; there is intentionally no repository-wide license override.
