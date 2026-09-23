# scripts/AGENTS.md

## Script Rules

- Use Bash with `set -euo pipefail` for project helper scripts.
- Scripts should work when launched from the repository root.
- Keep scripts non-destructive by default.
- Prefer explicit commands over hidden magic. The output should be useful in CI
  and in a local terminal.
