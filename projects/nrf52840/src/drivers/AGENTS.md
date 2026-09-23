# src/drivers/AGENTS.md

## Driver Module Rules

- A driver should own one hardware surface or tightly related group of pins.
- Keep blocking startup work inside `begin()` and repeated work non-blocking.
- Do not hide pin choices in drivers. Accept pins through constructors or config.
- Keep display and sensor buffers explicit so RAM cost is reviewable.
