# Documentation map

This directory has one operational source of truth and a small historical archive. Do not treat every dated incident/checkpoint file as current instructions.

## Read in this order

1. `HANDOFF.md` — current project state, known gaps and next-session starting point.
2. `WORKFLOW.md` — canonical ARTWORK -> PREPARE -> PRINT architecture and fail-fast rules.
3. `GOLDEN_LIVE_FLOW.md` — exact operator/live execution contract once a job is ready.
4. `HARDWARE.md` — current physical machine configuration.
5. `CALIBRATION.md` — current measured envelope, orientation and pen Z values.
6. `SAFETY.md` — non-negotiable machine/transaction safety rules.
7. `PREPARE_CLI.md` — offline job preparation contract.
8. `ROADMAP.md` — remaining engineering work.

`config/kobra2_neo_pen.toml` is the executable source of truth for the current plotting profile. Documentation must match it; if code, docs and profile disagree, stop before physical execution and resolve the mismatch in a dedicated maintenance change.

## Historical only

- `GOLDEN_CHECKPOINT_2026-09-28.md`
- `INCIDENT_2026-09-28_SLOW_LIVE_START.md`

These files are retained for evidence and lessons learned. They are not operator runbooks and must not override the current documents above.

## Repository hygiene rule

Generated artwork must enter this project as one normal source file (for example one `.svg`) or already exist on the host as one normal file. Do not transport artwork through hand-built base64/gzip chunks, task JSON payloads or a series of partial repository files. If a single-file transfer path is unavailable, stop and choose a proper transfer mechanism before PREPARE.
