# axp2101_pmic review checklist

## Schematic Sign-Off
- czy `VBUS` nigdy nie przekracza `5.5V`, takze przy `Voc` panelu solarnego?
- czy `BAT` ma lokalne `1uF` przy pinie?
- czy `VBUS` ma lokalne `10uF + 2.2uF` przy pinie?
- czy `VREF` ma lokalne `1uF`?
- czy `VSYS` ma lokalne kondensatory i tor `SW -> L_PMIC_SW -> VSYS` nie zostal przypadkiem usuniety?
- czy `VIN1..4`, `ALDOIN` i `BLDOIN` nie sa zostawione wiszace?
- czy `DCDC1` ma komplet: `LX1`, inductor, output cap i sprzezenie zwrotne z output node?
- czy nieuzywane `DCDC` maja podlaczone tylko `VIN`, a `LX/FB` zostaja otwarte bez output cap?
- czy nieuzywane `LDO` zostaja otwarte bez lokalnych output cap?
- czy `TS` jest rozwiazane swiadomie:
  - `10k NTC`
  - `10k` do `GND`
  - albo floating z jawnie wylaczona detekcja temperatury w konfiguracji PMIC
- czy `IRQ` ma pull-up do zamierzonej domeny logicznej:
  - domyslnie `10k` do `VRTC` dla always-on
  - albo `4.7k` do `3V3`, jesli host ma czytac `IRQ` bezposrednio
- czy `I2C` ma efektywnie `2.2k` pull-upy zgodne z datasheetem?
- czy `VBackup` jest swiadomie zwiazany z `VRTC` albo jawnie pozostawiony `NC`?
- czy w projekcie jest jawna decyzja, jak `PWRON` ma uruchomic system z samej baterii?

## Layout Sign-Off
- czy petla `LX1` jest kompaktowa?
- czy `L_PMIC_SW` siedzi blisko pinu `SW`?
- czy `EP` ma sensowne odprowadzenie do `GND`?
- czy `BAT`, `VSYS`, `VBUS`, `VMID` i `SW` maja szerokie prowadzenie i dobra referencje masy?
- czy `I2C`, `IRQ` i `TS` nie przechodza przez obszar przelaczajacy bucka?
- czy `C_PMIC_VBUS*`, `C_PMIC_VREF`, `C_PMIC_BAT`, `C_PMIC_SYS*`, `C_PMIC_VIN*`, `C_PMIC_ALDOIN`, `C_PMIC_BLDOIN` siedza naprawde przy pinach?

## System Integration
- czy board ma osobny front-end dla `USB + solar`, skoro `AXP2101` ma jedno wejscie?
- czy przy starcie tylko z baterii projekt ma swiadomy plan dla `PWRON` / custom power-on?
- czy `ESP32-S3` jest zasilane z wyjscia o wystarczajacej wydajnosci pradowej?
- czy decyzja o osobnym protection IC dla `18650` jest jawna?
- czy probki `AXP2101` maja potwierdzone factory defaults dla startup sequence i napiec wyjsciowych?
- czy bateria ma przygotowany profil dla `E-gauge 3.0`, jesli projekt ma raportowac sensowne `SoC`?

## Bring-Up
- odczytac po `I2C` status `VBUS_GOOD`, napiecie `VBAT`, `VBUS`, `VSYS`
- zweryfikowac domyslny prad ladowania i `VINDPM`
- zweryfikowac, ze `DCDC1` startuje z oczekiwanym napieciem `3V3`
- zweryfikowac `IRQ` low-battery / charger events
- potwierdzic, czy `TS` jest interpretowane zgodnie z zamierzonym hardwarem

## Open Items Before Production
- potwierdzic finalny land pattern `QFN-40 5x5`
- dobrac ostateczne parametry `DCDC1`
- dopiac finalna topologie `USB + solar` przed `VBUS`
- zdecydowac czy `TS` idzie w `NTC`, stala kwalifikacje czy inny sygnal
- zdecydowac, czy potrzebne sa dodatkowe aux rails i ktore z nich sa rzeczywiscie wlaczone w konfiguracji PMIC
