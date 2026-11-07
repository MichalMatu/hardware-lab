# ESP32-S3 Edge AI Lab

Status: **early project definition / scaffold**. Architecture, use cases and implementation scope remain open.

An independent, reusable exploration of lightweight on-device intelligence for ESP32-S3 systems. The recommended long-term direction is **a small sensor/time-series intelligence module** that can interpret observations and publish insights (anomalies, short-horizon forecasts, changes in behavior, and diagnostics) without taking ownership of hardware or automation policy.

See [Project direction](docs/PROJECT_DIRECTION.md) for the evolving vision and BloomML donor policy, and [reuse research](docs/REUSE_RESEARCH.md) for practical ready-made code/library candidates.

## Working principles

- Keep experiments independent of GrowClip and other application-specific automation engines.
- Optimize for real usefulness: compare simple signal-processing/statistical baselines with TinyML rather than assume a neural model is better.
- Favor small internal-DRAM overhead. Evaluate PSRAM/Flash use, performance, allocation safety and latency on real hardware; zero new DRAM is not guaranteed.
- Keep data acquisition, time-series processing, inference, results and application adapters separated.
- Let applications decide what to do with an insight. The module must not directly actuate devices.
- Train/evaluate on a host where appropriate; device-side inference should work offline.
- Treat source-qualified tests and measurements as prerequisites for production integration.

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
| `docs/` | Exploratory notes, decisions and source references |

## Scope status

This remains **documentation and directory scaffolding only**: no firmware, model, dependencies or GrowClip changes are introduced. Frameworks, interfaces, model families, target boards and acceptance thresholds have not been selected.

BloomML (`MichalMatu/bloomml`) is a **potential upstream donor** and is **still being actively developed**. Do not fork/synchronize it wholesale or assume its current artifacts are reusable without independent review. See the [donor notes](docs/PROJECT_DIRECTION.md#bloomml-as-a-potential-donor).
