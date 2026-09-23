# boards/AGENTS.md

## Board Definition Rules

- Treat files in this directory as low-level platform configuration.
- Do not edit the board JSON or Arduino variant unless the change is directly
  about pin mapping, memory, upload behavior, USB identity, or board support.
- After any board change, run `pio run` and update the relevant hardware docs.
- Keep local board definitions self-contained so the project remains standalone.
