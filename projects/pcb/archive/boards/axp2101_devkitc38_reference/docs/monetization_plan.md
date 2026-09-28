# JLCPCB SMT BOM & Strategia Monetyzacji AXP2101 HAT

Przygotowałem dla Ciebie strategię doboru części pod usługę **JLCPCB PCBA (SMT)** oraz listę brakujących elementów, które odróżniają zwykły prototyp od w pełni komercyjnego "Dev Boarda" gotowego do sprzedaży.

---

## 1. Rekomendowany BOM (Części dla JLCPCB)

Aby uniknąć opłat "Extended Part" (ok. 3$ za każdą niestandardową szpulę), musimy celować w części typu **"Basic"** z katalogu LCSC.

| Element | Wartość / Obudowa | Rola | LCSC Part # (Przykładowy) | Typ (Basic/Extended) |
| :--- | :--- | :--- | :--- | :--- |
| **PMIC** | AXP2101 (QFN-40) | Główny sterownik | **C2880796** | Extended (Kluczowy)* |
| **C_PMIC_SYS**, **C_DCDC** | 22uF / 1206 (X5R, >10V) | Filtracja dużej mocy | **C13585** | **Basic** |
| **C_VBUSA** | 10uF / 0805 (X5R, >10V) | Filtracja wejściowa 5V | **C15850** | **Basic** |
| **C_Decoupling** | 2.2uF / 0603 (X5R/X7R, 10V) | Rozsprzęganie LDO i VIN | **C23630** | **Basic** |
| **C_VREF, C_BAT** | 1uF / 0603 (X5R/X7R, 10V) | Referencje i Bateria | **C15849** | **Basic** |
| **R_Pullups** | 10k / 0603 | Rezystory I2C, TS | **C25804** | **Basic** |
| **R_IRQ** | 4.7k / 0603 | Rezystor IRQ | **C23162** | **Basic** |
| **L_DCDC, L_SW** | 1uH / 1210 (3.2x2.5mm) | Cewki prądowe (Isat > 3A) | **C372439** (Sunlord) | Extended (Kluczowy)* |
| **Q_CHG (NMOS)** | 2N7002 / SOT-23 | Tranzystor wskaźnika | **C8545** (S8050) / C8598 | **Basic** |
| **D_CHG** | 1N4148W / SOD-123 | Dioda | **C8598** | **Basic** |
| **SW_PWRON** | TL3342 (SMD Push) | Przycisk zasilania | **C318884** | Extended (Tanie) |

> [!NOTE]
> *Układy scalone (AXP2101) i specjalistyczne cewki mocy (SMD Power Inductors) w JLCPCB praktycznie zawsze są w kategorii "Extended". Opłata 3$ za załadowanie szpuli to standard, ale same części kosztują grosze, więc przy małoseryjnej produkcji opłaca się to znacznie bardziej niż lutowanie ręczne.*

---

## 2. Co brakuje do "Szybkiej Monetyzacji"? (Checklista Produktowa)

Jeśli chcesz to sprzedawać (np. na Tindie, Allegro, własnym sklepie) jako komercyjny moduł deweloperski, musisz spojrzeć na projekt oczami końcowego użytkownika (programisty), a nie tylko inżyniera-elektronika. 

Obecnie masz perfekcyjny elektronicznie zasilacz, ale słaby "produkt". Oto co musisz dodać do layoutu przed kliknięciem "Zamów":

### A. Mechanika i Bezpieczeństwo (Krytyczne)
1. **Otwory Montażowe (Mounting Holes):**
   Obecnie płytka "wisi" w powietrzu na pinach ESP32. Wymagane są minimum 2 (najlepiej 4) otwory M2.5 lub M3 na rogach płytki. Profesjonaliści nie kupią modułu, którego nie mogą przykręcić do obudowy!
2. **Złącze Baterii (Standard JST-PH 2.0):**
   Użytkownicy kupują baterie Li-Po w sklepach (Adafruit, Sparkfun, Botland) ze standardowym złączem `JST-PH 2.0mm`. Obecnie masz na schemacie ogólne "Conn_01x02". Zmień to na dedykowany footprint JST i KONIECZNIE oznacz na warstwie Silkscreen bieguny `+` i `-`.
3. **Zabezpieczenie przed odwrotną polaryzacją (Reverse Polarity Protection):**
   Programiści notorycznie podłączają baterie odwrotnie. Zastosowanie jednego małego tranzystora P-MOSFET w szeregu z baterią uchroni AXP2101 przed spaleniem. "Idiotoodporność" to główny atut drogich dev-boardów.

### B. Użyteczność (Wartość Dodana)
4. **Wyprowadzenie "Zapasowych" LDO (ALDO / BLDO):**
   Skoro chcesz to sprzedawać jako moduł AXP2101, zasilanie tylko ESP32 to marnowanie potencjału. AXP2101 ma mnóstwo konfigurowalnych wyjść LDO/DCDC. Wyprowadź je na mały, dodatkowy header (np. 1x6 pin) o rastrze 2.54mm z etykietami `ALDO1`, `ALDO2`, `BLDO1` itp., aby użytkownik mógł z nich zasilać zewnętrzne sensory.
5. **Opisy na Silkscreenie (Warstwa Opisowa):**
   - Dodaj piękne logo projektu.
   - Oznacz piny (IO32, 5V, GND, SDA, SCL) czytelną czcionką. Moduły bez opisów pinów trafiają do szuflady.
   - Dodaj URL do dokumentacji (np. `github.com/TwojeKonto/AXP2101-HAT`).

### C. Ekosystem Software'owy
6. **Biblioteka (Arduino / ESP-IDF):**
   Sprzęt to tylko 50% produktu. Do szybkiej monetyzacji musisz wrzucić na GitHuba prostą bibliotekę w C++/Pythonie, która po jednym wywołaniu `pmic.begin()` odpali I2C, zdejmie limit prądu z USB i włączy ładowanie baterii. Bez tego użytkownik utknie na czytaniu 100-stronicowego chińskiego datasheetu.
