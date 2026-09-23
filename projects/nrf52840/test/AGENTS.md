# test/AGENTS.md

## Test Rules

- Put PlatformIO tests under this directory.
- When firmware logic becomes testable without hardware, move it into small
  pure functions or classes and add native/unit tests.
- Hardware-dependent behavior should have documented manual verification steps
  until a board farm or hardware-in-loop setup exists.
