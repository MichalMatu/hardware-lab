# Profil: Moduł ładowania Li-Ion (TP4056 + DW01A)

Zrodla:
- `docs/modules/battery_power/references/TP4056-42-ESOP8_C16581.pdf`
- `docs/modules/battery_power/references/DW01A.pdf`
- `docs/modules/battery_power/references/FUXINSEMI-FS8205A_C908265.pdf`

## 1. Wymagania prądowe i termiczne
- **TP4056**: Ustawienie prądu ładowania rezystorem PROG (typowo 1.2k dla 1A). Maksymalna dyssypacja ciepła układu TP4056 wymaga podłączenia pada termicznego do dużego poligonu masy, w przeciwnym razie układ obniży prąd ładowania (thermal throttling).
- **DW01A (BMS)**: R1 ≥ 470 ohm i C1 ≥ 0.1 uF na pinie VDD (wg datasheet). Dodatkowo R2 ~ 2 k ohm na pinie CS (detekcja prądu/ładowarki).
- **Odcinanie obciążenia (Load Sharing):** Unikaj obciążania baterii w trakcie ładowania. Rekomendowane użycie tranzystora P-MOSFET (np. AO3401) i diody Schottky na linii V_USB w konfiguracji Power Path.

## 2. Layout
- Ścieżki od BAT+ i BAT- oraz pomiędzy układami zabezpieczającymi a złączem baterii muszą mieć szerokość co najmniej 30 mil dla 1A ciągłego prądu.
