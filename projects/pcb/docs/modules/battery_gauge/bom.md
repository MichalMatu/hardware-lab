# battery_gauge BOM

## Minimalny BOM
- `U_GAUGE`: `MAX17048G+`
  footprint: `Package_DFN_QFN:TDFN-8-1EP_2x2mm_P0.5mm_EP0.8x1.2mm`
- `C_GAUGE`: `0.1uF`
  footprint: `Capacitor_SMD:C_0603_1608Metric`
- `R_GAUGE_ALRT`: `10k`
  footprint: `Resistor_SMD:R_0603_1608Metric`
  uwaga: tylko jesli linia `ALRT` jest uzywana

## Cel poszczegolnych elementow
- `U_GAUGE`: pomiar napiecia ogniwa i obliczanie SoC przez `ModelGauge`
- `C_GAUGE`: lokalne odsprzeganie `VDD`
- `R_GAUGE_ALRT`: pull-up open-drain `ALRT` do logiki hosta

## Poza modulem
- pull-upy `SDA` i `SCL` naleza do osobnego modulu `i2c_bus`
- rail `VBAT_PROT / PACK+` dostarcza `battery_power` albo inny blok zrodla baterii
- konfiguracja alertow i ewentualne strojenie `RCOMP` naleza do firmware
