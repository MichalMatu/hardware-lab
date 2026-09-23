# Project Structure

This repository is intended to be a standalone firmware workspace.

## Source of Truth

- `platformio.ini` defines the build environments.
- `boards/nicenano.json` defines the local PlatformIO board.
- `boards/nicenano/variant.h` and `boards/nicenano/variant.cpp` define the
  Arduino pin map.
- `src/main.cpp` is the Arduino entry point and should stay small.
- `src/drivers/` contains hardware-facing modules.
- `src/app/` contains application coordination modules.
- `include/app_config.h` holds project-wide constants.
- `include/drivers/` and `include/app/` contain module interfaces.

## Quality Gates

- `scripts/format.sh --check` verifies C/C++ formatting.
- `pio run` verifies the firmware build and reports memory use.
- `pio check --fail-on-defect medium` runs static analysis.
- `scripts/quality.sh` runs the local gate used by CI.
- `Doxyfile` configures local API documentation output under `build/html/`.

## Local-Only Research

The `vendor/` directory is ignored by git. It can contain large cloned
reference projects, but the firmware must not depend on files under `vendor/`.

When a useful example is promoted from `vendor/`, copy the relevant source into
`src/`, `lib/`, or `examples/` and update `platformio.ini` if needed.

## Codex Instructions

`AGENTS.md` files are intentionally layered. The root file defines repository
rules, while nested files in `src/`, `include/`, `boards/`, `docs/`, `scripts/`,
`test/`, and `lib/` add subtree-specific rules.
