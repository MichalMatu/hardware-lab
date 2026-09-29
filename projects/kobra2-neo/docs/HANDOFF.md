# Kobra 2 Neo handoff — 2026-09-29

## Start here

For normal printing, read `CHAT_TO_PRINT.md` first. It is the canonical fast path for the user experience:

```text
attach image in ChatGPT
"drukuj"
-> ARTWORK
-> PREPARE
-> PRINT
-> DRAWING_STARTED
```

Do not redesign this flow in a new chat unless repository evidence proves it is broken.

## Current hardware/profile

Dedicated Anycubic Kobra 2 Neo pen plotter, stock Marlin baseline. Original hotend/heater/thermistor/fans are removed. Pen/marker is the active tool. Cylindrical proximity sensor is verified as `z_min` for Z homing.

Executable profile source of truth: `config/kobra2_neo_pen.toml`.

```text
hard envelope: X=3..223, Y=36..230 mm
normal drawing envelope: X=8..218, Y=41..225 mm
pen down: Z=2.97
pen up: Z=4.97
travel: 6000 mm/min
draw: 2400 mm/min
Z feed: 360 mm/min
orientation: flip Y only
end: M400
```

Idle `M105` hotend value `T:0.00` is expected because the thermistor is intentionally absent. Actual Marlin halt conditions remain terminal. Heater commands are forbidden.

## Implementation status

- Profile/live-runner drift closed by `7a3d510`; maintenance suite passed `33/33` plus compileall/doctor/diff-check.
- `DRAWING_STARTED` now corresponds to the first acknowledged `G1`; fixed by `ef7eea4`, tests passed.
- Optional end-to-end path reversal optimization `b358070` passed `35/35` locally but is not required by the canonical flow and was not pushed after two GitHub internal errors.
- `main` contains the proven chat-artwork workflow documentation.

## Proven chat -> ready path

The accepted botanical artwork from ChatGPT was transported as one normal SVG on `plot-inbox`.

```text
source commit: de057fd3a4250dd795e421cdff1d9d087f954b5d
source path: projects/kobra2-neo/inbox/botanical-chat-ingest.svg
source SHA-256: 8d3f26b2354b30a9cd0671e93c6c2137973ab23330cc0cecbd6cdb30ea28a945
Git blob: 772d7189578ed68a79b135179900b64470a1f6e1
```

Local Agent independently reconstructed that exact commit/path and verified the SHA before preparation.

```text
176 polylines
2018 points
2373 G-code commands
bounds X=22.57..203.43, Y=41.00..225.00 mm
safety PASS
G-code SHA-256: 18989085c5031f3e55e60b9103450d18435a0f872451e48b737cdc453bb2a8f4
PREPARE + kobra-live --validate-only: 2.780 s
RESULT: READY_TO_PRINT
```

## Proven physical path

Physical task:

```text
kobra2-neo-botanical-print-live-20260929-49
```

It consumed the already prepared immutable job, identified the printer dynamically, homed, and reached real streamed drawing progress (`DRAWING 800/2373` observed during this handoff). This proves the physical PRINT leg is operational; completion still requires terminal `COMPLETE_PEN_UP` evidence.

## Repository/branch hygiene

The repository intentionally has only:

```text
main          code/docs/profile
agent-control Local Agent control plane
plot-inbox    transient single-file artwork ingress
```

`plot-inbox` is intentional, not a disposable work branch. Do not create per-job development branches for normal printing. Prefer one reusable file path such as `projects/kobra2-neo/inbox/current.svg`; pin the commit SHA and source SHA-256 so the source remains immutable even when that path is replaced for a later job.

Never use base64/gzip chunk transport, split artwork files or task-JSON asset blobs.

## Fail-fast

One task = one responsibility. A stage failure stops that stage. One deterministic corrective retry is allowed after exact diagnosis. If that retry fails, stop; do not create speculative retry chains.

## New-chat starter prompt

Use the following as the first message in a fresh ChatGPT conversation and attach the image to that same message:

```text
Kontynuuj projekt Anycubic Kobra 2 Neo pen plotter z repo MichalMatu/hardware-lab.

Najpierw przeczytaj z aktualnego main:
- projects/kobra2-neo/docs/CHAT_TO_PRINT.md
- projects/kobra2-neo/docs/HANDOFF.md
- projects/kobra2-neo/docs/WORKFLOW.md
- projects/kobra2-neo/config/kobra2_neo_pen.toml
- projects/kobra2-neo/AGENTS.md

Załączony obraz jest dokładnie tym, co chcę narysować. DRUKUJ.

Użyj kanonicznego flow bez jego przeprojektowywania:
ARTWORK -> PREPARE -> PRINT.

Wymagania:
- użyj dokładnie załączonego obrazu, nie generuj zamiennika;
- jeden normalny SVG na plot-inbox, bez base64/chunków;
- przypnij commit i SHA-256 źródła;
- osobny offline PREPARE do READY_TO_PRINT;
- moje słowo "DRUKUJ" jest zgodą na pojedynczy fizyczny PRINT tego dokładnego przygotowanego joba;
- PRINT ma tylko zweryfikować job, znaleźć Kobrę przez M115, zrobić G28 X Y, G28 Z, pen-up i streamować przypięty G-code;
- żadnych zmian kodu, dokumentacji, pull/rebase/push ani tuningu podczas PRINT;
- nie pytaj ponownie o zgodę, jeśli automatyczne safety checks przejdą i obraz jest jednoznaczny;
- zgłoś DRAWING_STARTED dopiero po odpowiednim dowodzie z runnera;
- jeśli etap padnie: jedna deterministyczna poprawka i jeden retry; drugi fail = stop i konkretny blocker.

Cel: od tej wiadomości do fizycznego DRAWING_STARTED około 30 sekund w normalnych warunkach.
```

## Read order for maintenance/debugging only

1. `CHAT_TO_PRINT.md`
2. `HANDOFF.md`
3. `WORKFLOW.md`
4. `config/kobra2_neo_pen.toml`
5. `GOLDEN_LIVE_FLOW.md`
