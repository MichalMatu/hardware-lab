# Migration status

- Status: working hardware project; migrated without rebuilding by request.
- Canonical snapshot: former `main` branch at `0beafaaf13f6eaad100b5ecc8cd4a250efe76c15`.
- Preserved OLED variant: former `oled` branch at `940bbd2fe68379e9c88b0f6bc6b75482a7abd91f`, stored under `variants/oled/`.
- The two source branches had no common Git ancestor, so they were intentionally not merged.
- `main` remains the canonical USB NCM/Wi-Fi bridge implementation. The OLED variant preserves the SH1106 + EC11/BACK/CONFIRM diagnostic UI and its older configuration flow.
- No firmware build or device test was run during this migration because the project is already hardware-validated.
