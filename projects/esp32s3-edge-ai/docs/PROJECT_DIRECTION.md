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
- For GrowClip, prefer coherent cached sensor snapshots; Data Logger's microSD archive is optional and is not an implicit dependency. Raw samples, retained-on-change samples and rollups may have different statistical meanings. Never treat aggregate rollups as equivalent to raw observations.
- Independent host tools may load exported/consented historical datasets; MCU inference must not depend on host tools or a network connection.
- Keep results read-only/observational. A future application may use them as Nodeflow inputs, but the AI module does not dispatch actions.
- ML training and heavyweight experimentation can happen offline; the deployed device must not depend on cloud services.
- Dataset format, exact output fields, sampling cadence, thresholds and algorithm interface are **undecided**.

## Data and result semantics (design invariants, not a frozen schema)

A future *sample* boundary should distinguish source identity and unit, a numeric observation, monotonic capture time, data quality/freshness and, where known, operating context (e.g. configured actuator command and **separately** observed physical response). Do not infer a successful measurement from a scheduling tick, archive write or HTTP refresh. Sampling cadence, unit conversion and source-specific freshness thresholds remain open.

- Missing, invalid, repeated, out-of-order or delayed samples and reboot/time-base discontinuities must have explicit behavior. A new boot generally starts a new observation epoch; never silently join uptime-based windows across resets.
- Sample history may be collected in a fixed-capacity in-memory ring and/or supplied through an offline replay adapter. Reusing already captured measurements avoids competing sensor I/O. Choose retention/window sizes only after measuring realistic rates and resource costs.
- A result must distinguish **warming up**, **valid**, **stale/unavailable**, and **error** rather than exposing an unqualified score or anomaly boolean. A valid result should carry its source/window identity, generation/timestamp and freshness limit. Score threshold and model identity/version are needed when ML is involved; confidence is not automatically calibrated.
- Treat sensor health checks (e.g. stale/NaN), statistical change detection, forecast error, and learned anomaly scores as distinct methods even when surfaced through one future integration API.
- Do not allow downstream applications to treat an unavailable, warming-up or expired result as a valid positive trigger. GrowClip Nodeflow input conversion, scheduling and fail-closed behavior need their own contract review before integration.

For training/evaluation, distinguish **normal-only fitting data**, calibration data and held-out test traces. Split by time/trajectory and operating regime, prevent look-ahead leakage, and record ground truth/source when available. Synthetic faults test robustness but do not alone validate real-world detection quality. Define an acceptable false-alarm burden and usefulness against a simple non-ML baseline before model selection. Retention, export security and data privacy remain to be decided.

## Resource and hardware priorities

Internal DRAM is scarce. Design for **minimal *measured* additional internal DRAM**, not an unproven zero-allocation claim.

- Investigate PSRAM for bounded observation windows and suitable temporary buffers, Flash for model assets/weights, and minimal internal state where required.
- Review memory capabilities for task control blocks, task stacks, DMA and code paths with disabled Flash cache; do not assume that every buffer can safely move to PSRAM.
- Evaluate alternatives to an extra permanent worker if scheduling and isolation can be achieved safely with less overhead. Never run long inference or blocking I/O inside time-critical control/heartbeat paths.
- Measure internal free heap, largest free block, boot/runtime minima and peak allocation, PSRAM allocation/free and largest block, Flash footprint, inference duration and control-loop jitter on the actual target configuration. Compare like-for-like idle and concurrent load (network/TLS, storage, display as applicable) before and after enabling the module.
- Distinguish resident model weights, tensor/scratch buffers, FreeRTOS stack/TCB, allocators and temporary buffers. A PSRAM stack or arena is not automatically safe on paths that can run with Flash cache disabled or require specific DMA/internal-memory capabilities.
- GrowClip's current build uses Arduino/PlatformIO over ESP-IDF; an ESP-IDF component example is a compatibility candidate, not drop-in proof. Keep the portable core independent of both build systems.
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

## Readiness gates for a future implementation

These are **evidence categories**, not pre-approved thresholds or a committed sprint:

1. A fixed deterministic baseline that handles missing/duplicate/out-of-order/gapped data and changes across reboots.
2. Replay on representative real or controlled data, with false positives, missed events, detection delay and utility versus a simple threshold/slope baseline; protect hold-out traces from training and tuning.
3. Target-specific DRAM/PSRAM/Flash/CPU/time accounting, including first startup and any worker stack/TCB, and a no-silent-DRAM-fallback allocation policy.
4. Safe lifecycle, concurrent network/storage stress, watchdog/heartbeat impact and model/runtime failure paths; cold boot, shutdown, disabled mode and no sensor/SD available.
5. License and precise commit/component provenance for any donor code, with independent host and hardware verification before downstream reuse.
6. A separate application-adapter contract review (e.g. GrowClip's numeric providers, sample freshness and action gating), with AI-only shadow observation first.

No gate is marked passed yet; no hardware tests or firmware changes were made as part of this documentation.

## Still open

The initial use case, selection of hardware/profile, data source and sampling cadence, data collection/retention/privacy policy, sample and result schemas, model format, label/threshold policy, training method, component-license review, DRAM/PSRAM/Flash ceilings, runtime scheduling and integration order all remain undecided.

Avoid turning these notes into a detailed spec until those choices have been discussed and measured.
