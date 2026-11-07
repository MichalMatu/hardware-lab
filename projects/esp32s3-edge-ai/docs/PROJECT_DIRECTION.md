# ESP32-S3 Edge AI Lab — project direction

Status: **working vision, not an implementation specification**  
Updated: 2026-11-07  
Workspace: `MichalMatu/hardware-lab/projects/esp32s3-edge-ai/`

## Purpose

Explore an independent, reusable **on-device intelligence layer for time-series and sensor data** on ESP32-S3 class hardware. Instead of building one growbox anomaly detector, aim for a small toolkit that can later serve GrowClip and other embedded projects through narrow adapters.

The value is *earlier, context-aware information*, not replacing deterministic automation. The module should observe available data, derive useful signals and report its findings. Control policy, safety and physical outputs remain the responsibility of the host application.

## Recommended future-facing direction

**Predictive monitoring and diagnostics for edge devices** is the preferred research direction.

Candidate capabilities, to be validated rather than promised:

1. **Change/anomaly detection** — unusual jumps, drifting baselines, stuck readings, unexpected operating patterns.
2. **Short-horizon forecasting** — temperature/humidity/CO2 trends and likelihood of crossing an operational threshold soon.
3. **Behavior and effectiveness monitoring** — whether an observed device response appears consistent with an action, e.g. ventilation followed by expected sensor trends. Observational correlation is not proof of causality.
4. **Sensor quality insights** — stale values, invalid measurements, implausible dynamics or disagreement between sensors.
5. **Compact learned features** — only when small neural models provide a measured advantage over statistical or deterministic methods.

Examples are illustrative, not an approved product backlog. Avoid assuming every use case needs neural inference; some may be served best by bounded statistics.

## Proposed conceptual boundaries (not frozen APIs)

```text
application / hardware sensors / logs
             |
        sample adapter
   (value, time, quality, source)
             |
    bounded observation window
             |
   analysis / inference algorithms
             |
    result + validity + context
             |
   application-specific adapter
  (GrowClip Nodeflow, UI, logs, ...)
```

- The core should be portable and testable on a host without ESP32 hardware.
- ESP32-S3-specific storage, memory and acceleration are platform concerns.
- Data sources are supplied by the application; never require a new direct hardware polling path or microSD access for every inference.
- Keep results read-only/observational. A future application may use them as Nodeflow inputs, but the AI module does not dispatch actions.
- ML training and heavyweight experimentation can happen offline; the deployed device must not depend on cloud services.
- Dataset format, exact output fields, sampling cadence, thresholds and algorithm interface are **undecided**.

## Resource and hardware priorities

Internal DRAM is scarce. Design for **minimal *measured* additional internal DRAM**, not an unproven zero-allocation claim.

- Investigate PSRAM for bounded observation windows and suitable temporary buffers, Flash for model assets/weights, and minimal internal state where required.
- Review memory capabilities for task control blocks, task stacks, DMA and code paths with disabled Flash cache; do not assume that every buffer can safely move to PSRAM.
- Evaluate alternatives to an extra permanent worker if scheduling and isolation can be achieved safely with less overhead. Never run long inference or blocking I/O inside time-critical control/heartbeat paths.
- Measure internal free heap, largest free block, peak allocation, PSRAM, Flash and inference duration on the actual target configuration.
- ESP32-S3 offers useful vector instructions, **not a dedicated general-purpose NPU**. SIMD/ESP-NN and framework choices must be benchmarked rather than assumed faster.
- In the absence of reliable/fresh observations or an accepted model, return an unavailable result; do not fabricate a confident score or command.

Initial comparison candidates: simple signal/statistical methods; a tiny C/C++ inference runtime; and an ESP32-S3-optimized path if its real resource/accuracy trade-off warrants it.

## BloomML as a potential donor

Source project: [MichalMatu/bloomml](https://github.com/MichalMatu/bloomml)  
Reference examined: `main@d5044b712b9f7b56278c31f5770d853836089071` (2026-11-07 snapshot).

**BloomML is a separate project still under active development.** It must remain independent and authoritative for its own application. Revisit its current main/architecture before any reuse; the reference SHA is for traceability, not a mandate to pin development there.

Promising material to evaluate selectively:

| BloomML location | Possible reusable idea | Qualification needed |
| --- | --- | --- |
| `components/emlearn_runtime/` | Lightweight header-only inference runtime | Upstream MIT notice/provenance, memory placement, thread safety, target build |
| `tools/ml/export_model.py` and `tools/ml/climate_model_artifact.py` | Reproducible model export, metadata, version/hash checks | Separate generic export from growbox-specific feature/schema contracts |
| `tools/ml/climate_trace.py`, `tools/ml/climate_replay.py`, `tools/ml/generate_climate_runtime_parity.py` | Record/replay and cross-language parity practices | Create minimal application-neutral formats and golden tests |
| `lib/environment_control/src/climate/ClimateTrendEstimator.*` | Bounded time-series trend estimation | Extract only portable mathematics and revalidate window/cadence behavior |
| `docs/ML_DECISION_REPORT.md`, `docs/MODEL_PIPELINE.md` | Research findings and decision discipline | Reuse the lessons, not product-specific control assumptions |

**Do not import wholesale:** BloomML's entire growbox controller, climate-v6 feature contract, fixed 44→32→32→6 weights/model, output policy, generated artifacts or ESP-IDF runtime wiring. Its own research found no sufficient reason to promote the tested ML controller over deterministic Rule control; that outcome should inform our benchmarks, not be generalized to every predictive task.

The existing `emlearn` runtime is a candidate, **not proof of ESP32-S3 SIMD acceleration**. In BloomML's `ModelRuntime.h`, generated inference uses fixed global scratch and requires serialized calls. Check actual linker placement and DRAM usage before adoption; an application-neutral library should explicitly own/allocate scratch safely.

### Donor reuse procedure

1. Re-check the donor's current `main`, license notices, source and associated tests before selecting code.
2. Record the exact source path, commit and license for every adopted piece; preserve required upstream attribution.
3. Copy/extract **only** a justified portable component or design idea, not a live link to a moving upstream.
4. Adapt behind Hardware Lab-owned interfaces and add host tests, replay/parity fixtures and ESP32-S3 resource benchmarks.
5. Keep BloomML and Hardware Lab independently buildable. No automatic synchronization or bidirectional coupling.

No donor material has been copied or integrated by this documentation change.

## Possible learning path, not a committed roadmap

- Define one meaningful sensor history/replay experiment and a deterministic baseline.
- Prototype one portable detector or forecast with an explicit no-data state.
- Compare against one small ML alternative only when the baseline has a quantified limitation.
- Qualify PSRAM/DRAM/Flash/latency on an ESP32-S3, including concurrency and lifecycle behavior.
- Consider a thin GrowClip adapter **only after** a reusable module shows value and safe bounded behavior.

## Still open

The initial use case, selection of hardware/profile, data collection and retention policy, sample contracts, model format, training method, license review, memory ceilings, runtime scheduling and integration order all remain undecided.

Avoid turning these notes into a detailed spec until those choices have been discussed and measured.
