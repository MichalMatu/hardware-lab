# src/AGENTS.md

## Source Layout

- Keep `main.cpp` focused on Arduino `setup()` and `loop()` orchestration.
- Put hardware-facing code in `src/drivers/`.
- Put application coordination code in `src/app/` when logic grows beyond a
  small sketch.
- Keep feature-specific behavior in `src/features/` when it is independent from
  low-level drivers.

## Embedded C++ Rules

- Target C++11-compatible code unless `platformio.ini` explicitly changes the
  standard.
- Prefer `constexpr`, `const`, fixed-size arrays, and explicit types.
- Avoid exceptions, RTTI, hidden heap ownership, recursion, and dynamic
  allocation in update loops.
- Use `uint32_t now = millis()` once per loop and pass time into modules when
  possible.
- Hardware modules should expose `begin()` and non-blocking `update(now)` style
  APIs.

## Anti-patterns

- Do not add long blocking delays after setup.
- Do not scatter pin numbers through feature code.
- Do not make Serial logging required for normal firmware operation.
- Do not add global mutable state without clear hardware ownership.
