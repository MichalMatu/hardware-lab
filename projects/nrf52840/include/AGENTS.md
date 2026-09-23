# include/AGENTS.md

## Header Rules

- Headers define project interfaces, compile-time configuration, and shared
  types. Keep implementation in `src/` unless it must be inline or `constexpr`.
- Use `#pragma once` for project headers.
- Do not define non-`constexpr` global objects in headers.
- Keep pin constants readable and close to the hardware naming used by the
  board variant.
- Include only what the header needs; prefer forward declarations where they
  keep dependencies clear.
