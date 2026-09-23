# AGENTS.md

## Repository Scope

- This is a standalone PlatformIO firmware project for nRF52840 SuperMini /
  nice!nano-class boards using the Arduino nRF52 framework.
- Keep the project buildable from a fresh clone. Do not make firmware depend on
  ignored `vendor/` snapshots or local `.pio/` state.
- Do not add or preserve redundant backward-compatibility paths. When replacing
  an internal API, configuration shape, flag, request field, or contract,
  migrate current callers and remove old inputs, branches, constants, comments,
  and fallbacks in the same change unless explicitly requested otherwise.

## Required Gates

- Run `scripts/format.sh --check` after changing C/C++ source, headers, board
  variants, or local libraries.
- Run `pio run` after changing firmware, PlatformIO config, board definitions,
  or dependencies.
- Run `pio check --fail-on-defect medium` after changing C/C++ code.
- Run `scripts/quality.sh` before commits that should be pushed.

## Firmware Rules

- Prefer small modules with explicit ownership over growing `src/main.cpp`.
- Keep pin assignments and board constants centralized in `include/app_config.h`
  or a more specific config header.
- Prefer static or stack storage. Avoid heap allocation in steady-state loops.
- Do not use Arduino `String` in hot paths; prefer fixed text, literals, or
  caller-owned buffers.
- Keep `loop()` non-blocking. Use `millis()` scheduling instead of long
  `delay()` calls once startup is complete.
- Treat RAM, flash, GPIO current, and power states as design constraints.
- Document every new physical wiring assumption in `docs/hardware/`.

## Dependency Rules

- Prefer PlatformIO-managed dependencies in `platformio.ini`.
- Add a new firmware dependency only when it materially reduces risk or
  complexity.
- Do not vendor full upstream projects into tracked source. Keep research clones
  under ignored `vendor/`.

## Git Rules

- Do not revert user changes unless explicitly asked.
- Stage only files that belong to the requested change.
- Keep generated build outputs out of git.
