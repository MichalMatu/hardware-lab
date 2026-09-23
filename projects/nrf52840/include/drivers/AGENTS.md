# include/drivers/AGENTS.md

## Driver Header Rules

- Driver headers may include hardware library headers only when the class owns
  that library type.
- Keep constructors explicit about pins, bus choices, sizes, and update rates.
