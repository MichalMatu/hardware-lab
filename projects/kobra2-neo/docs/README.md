# Documentation map

Operational docs are intentionally small. Historical incident/checkpoint files are evidence only and must not override the current runbooks.

## Normal printing — read in this order

1. `CHAT_TO_PRINT.md` — canonical fast path for `attach image -> drukuj -> DRAWING_STARTED`.
2. `HANDOFF.md` — current verified state, branch hygiene and fresh-chat starter prompt.
3. `WORKFLOW.md` — staged ARTWORK -> PREPARE -> PRINT architecture and fail-fast rules.
4. `GOLDEN_LIVE_FLOW.md` — exact live execution/evidence contract.
5. `config/kobra2_neo_pen.toml` — executable plotting profile source of truth.

## Reference / maintenance

- `HARDWARE.md` — physical machine configuration.
- `CALIBRATION.md` — measured envelope, orientation and pen Z values.
- `SAFETY.md` — non-negotiable machine/transaction safety rules.
- `PREPARE_CLI.md` — offline job preparation contract.
- `ROADMAP.md` — remaining engineering work.

If code, docs and profile disagree, stop before physical execution and fix the mismatch in a dedicated maintenance change. Do not mix maintenance with artwork preparation or printing.

## Historical only

- `GOLDEN_CHECKPOINT_2026-09-28.md`
- `INCIDENT_2026-09-28_SLOW_LIVE_START.md`

## Repository hygiene

Normal artwork ingress uses branch `plot-inbox` and one normal SVG file. Prefer a reusable path such as `projects/kobra2-neo/inbox/current.svg`; pin the exact commit plus source SHA-256 for each job. Never use hand-built gzip/base64 chunks, split artwork files or task-JSON asset payloads.

Normal repository branches are intentionally limited to `main`, `agent-control` and `plot-inbox`.
