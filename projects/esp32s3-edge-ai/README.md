# ESP32-S3 Edge AI Lab

Status: **scaffold only**. The scope is intentionally open.

A standalone workspace for experimenting with lightweight on-device data analysis and possible AI inference on ESP32-S3 hardware. The aim is to explore what is practical before deciding which capabilities, if any, should be integrated into other firmware projects such as GrowClip.

## Working principles

- Keep experiments independent from application-specific automation engines.
- Measure internal DRAM, PSRAM, execution time and runtime impact rather than assume resource costs.
- Separate reusable algorithms from ESP32-S3-specific integration.
- Prefer simple, testable approaches first; do not require a neural network to call an experiment useful.
- Do not treat an experiment as production-ready without evidence.

## Layout

| Path | Intended purpose |
| --- | --- |
| `include/` | Public interfaces, when defined |
| `src/core/` | Portable algorithms and model-independent logic |
| `src/platform/esp32s3/` | ESP32-S3 platform adapters |
| `models/` | Future model assets and metadata, if needed |
| `examples/` | Small isolated demonstrations |
| `benchmarks/` | Resource/performance experiments |
| `tests/` | Unit and integration tests |
| `docs/` | Exploratory notes and decisions |

## Not decided yet

Use cases, input datasets, frameworks, model formats, API surface, hardware targets, validation thresholds and integration strategy are **open questions**. See [docs/README.md](docs/README.md).

No firmware, model, build system, dependencies or GrowClip integration is introduced by this scaffold.
