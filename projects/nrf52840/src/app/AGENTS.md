# src/app/AGENTS.md

## Application Module Rules

- Keep application modules independent from concrete pins when possible.
- Coordinate drivers through small methods such as `begin()` and `update(now)`.
- Do not put direct I2C/SPI/GPIO calls here unless the module is being promoted
  into `src/drivers/`.
