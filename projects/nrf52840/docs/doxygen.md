# Doxygen

Doxygen is used for lightweight API documentation of stable project interfaces.
The source of truth is still the code and operational documentation under
`docs/`.

## Install

On macOS with Homebrew:

```sh
brew install doxygen
```

## Generate

From the repository root:

```sh
doxygen Doxyfile
```

Generated HTML is written to:

```text
build/html/index.html
```

`build/` is ignored by git, so generated API docs are not committed.

## Scope

Document public headers when they describe a stable contract:

- `include/app/device_menu.h`
- `include/app/button_input.h`
- `include/drivers/oled_panel.h`
- `include/drivers/button_panel.h`
- `include/app_config.h`

Avoid line-by-line comments for private implementation details. Prefer comments
that explain ownership, state transitions, timing, hardware assumptions, and
resource constraints.
