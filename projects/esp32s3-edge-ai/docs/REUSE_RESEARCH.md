# Reuse research — ESP32-S3 Edge AI

Status: **research / candidate donor inventory, no implementation selected**  
Reviewed: 2026-11-07  
Target: reusable ESP32-S3 sensor/time-series intelligence, with scarce internal DRAM and possible PSRAM.  
External code has **not** been imported or benchmarked on this device.

## Executive selection

**Start with BloomML + emlearn + ESP-DSP, not a large new framework.**

- **BloomML** provides working reference ideas for trend estimation, trace/replay, schema checks, portable C++ logic, and Python/C++ parity. It is an **active, moving donor**; borrow only small validated pieces after re-checking main.
- **emlearn** provides low-overhead portable C inference for trained models, including Gaussian mixture / Mahalanobis-style anomaly scoring. Its existing model tooling is a fast path to real, explainable experiments; no ESP32-S3 SIMD benefit is guaranteed.
- **ESP-DSP** provides Espressif's optimized filters, FFT, vector and matrix routines. Reuse only relevant functions (initially filters/vector maths); signal processing is useful without a neural model.
- **ESP-NN** deserves a later controlled test of ESP32-S3 vector acceleration with quantized models via TFLite Micro. A faster convolution benchmark alone does not justify a heavier production stack.

A viable early result is a **read-only score/forecast service** that consumes existing sensor snapshots, reports freshness/validity and never owns physical outputs. Keep framework switching and model selection behind an adapter.

## Candidate matrix

Effort is an architectural estimate for a *small isolated lab integration*, not a tested measurement. Licenses apply to each upstream's own original sources; inspect bundled/third-party and generated assets before copying.

| Priority | Project / source | What to reuse | License (reported upstream) | Reuse effort | Main caveat |
| --- | --- | --- | --- | --- | --- |
| P0 | [MichalMatu/bloomml](https://github.com/MichalMatu/bloomml) | `ClimateTrendEstimator`, trace/replay approach, fixed goldens and model-export discipline | MIT | Low–medium for focused extraction | Growbox climate-v6 data/schema/control are NOT generic; project is changing |
| P0 | [emlearn/emlearn](https://github.com/emlearn/emlearn) | Portable C inference and model export, anomaly-score models | MIT | Low for a standalone host demo; medium for ESP32-S3 qualification | No assured S3 SIMD; ensure scratch/thread safety and actual RAM placement |
| P0 | [espressif/esp-dsp](https://github.com/espressif/esp-dsp) | IIR/FIR, vector math, optional FFT/filters, reference examples | Apache-2.0 | Medium | ESP-IDF component; avoid importing entire subsystem into an Arduino/PlatformIO app just for a few operations |
| P1 | [sipeed/TinyMaix](https://github.com/sipeed/TinyMaix) | Alternative tiny INT8/FP32 inference core and memory comparison | Apache-2.0 | Medium | Validate supported layers, static buffers and Xtensa implementation; benchmark against emlearn rather than defaulting to it |
| P1 | [espressif/esp-nn](https://github.com/espressif/esp-nn) | Optimized S3 INT8 NN kernels, assembly/reference parity methodology | Apache-2.0 | Medium–high | Kernel library, not a complete stand-alone anomaly product; intended integration with TFLite Micro |
| P2 | [espressif/esp-dl](https://github.com/espressif/esp-dl) | Quantized ESP-specific model format/runtime, model tooling, S3 accelerated inference | MIT for main upstream source | High | More ecosystem/model conversion overhead; evaluate model-specific memory and framework compatibility |
| P2 | [espressif/esp-tflite-micro](https://github.com/espressif/esp-tflite-micro) | Espressif reference application integrating TFLM + ESP-NN | Apache-2.0 for component/examples; third-party notices vary | High | Example benchmark uses **internal RAM**; cannot assume equal speed with PSRAM arena |
| Host only | [online-ml/river](https://github.com/online-ml/river) | Streaming statistics, drift/anomaly algorithms as evaluation references | BSD-3-Clause | Low for Python evaluation, high to port | Python framework is NOT MCU firmware |
| Host only | [numenta/NAB](https://github.com/numenta/NAB) | Anomaly benchmark runner, labeled series and detection scoring | MIT repository; check individual dataset provenance | Low for offline reference | Public datasets differ from growbox signals; they do not replace domain validation |
| Reference | [edgeimpulse/inferencing-sdk-cpp](https://github.com/edgeimpulse/inferencing-sdk-cpp) | Signal-processing/inferencing design ideas | BSD-3-Clause-Clear and mixed component licenses | Medium–high | Broader SDK and license/provenance surface than first prototype needs |
| Deprioritize | [eloquentarduino/micromlgen](https://github.com/eloquentarduino/micromlgen) | Simple generated C/C++ classifiers | MIT | Low | Archived 2024; emlearn is a more maintainable first candidate |

## Concrete donor touchpoints

### BloomML (internal source)

Reference observed at `main@d5044b712b9f7b56278c31f5770d853836089071`; refresh before copying.

- [lib/environment_control/src/climate/ClimateTrendEstimator.cpp](https://github.com/MichalMatu/bloomml/blob/main/lib/environment_control/src/climate/ClimateTrendEstimator.cpp) — bounded samples and slope. Candidate for math-only extraction.
- [lib/environment_control/src/climate/ClimateFeatureEncoder.cpp](https://github.com/MichalMatu/bloomml/blob/main/lib/environment_control/src/climate/ClimateFeatureEncoder.cpp) — freshness, validity, range checking and normalization **patterns**, not the fixed 44-feature climate contract.
- [components/emlearn_runtime](https://github.com/MichalMatu/bloomml/tree/main/components/emlearn_runtime) — a small header-only emlearn subset; note this component carries its own upstream MIT notice.
- [lib/environment_control/src/ModelRuntime.h](https://github.com/MichalMatu/bloomml/blob/main/lib/environment_control/src/ModelRuntime.h) — current generated model has **fixed global scratch buffers and requires serialized invocation**. Do not blindly copy this memory model.
- [tools/ml/export_model.py](https://github.com/MichalMatu/bloomml/blob/main/tools/ml/export_model.py) — deterministic model/manifest/golden export.
- [tools/ml/climate_replay.py](https://github.com/MichalMatu/bloomml/blob/main/tools/ml/climate_replay.py) and [tools/ml/generate_climate_runtime_parity.py](https://github.com/MichalMatu/bloomml/blob/main/tools/ml/generate_climate_runtime_parity.py) — reproducible replay and Python/C++ consistency.
- [docs/ML_DECISION_REPORT.md](https://github.com/MichalMatu/bloomml/blob/main/docs/ML_DECISION_REPORT.md) — evidence that the evaluated climate ML policies did not qualify for active control; preserve rule/safety ownership.

**Donor rule:** track exact source path, commit, license and upstream notice for a chosen module. Copy only reviewed portions, make interfaces project-neutral, test them independently. Do not synchronize all of BloomML or import its saved model/weights as an anomaly detector.

### emlearn

- [Repository and API](https://github.com/emlearn/emlearn).
- [Anomaly-detection documentation](https://emlearn.readthedocs.io/en/latest/anomaly_detection.html) lists **Gaussian Mixture Models** and **Mahalanobis distance / EllipticEnvelope**; output is an anomaly **score** whose threshold must be validated.
- Fit/calibrate on representative **normal** sensor data offline. Define false-alarm and missed-detection trade-offs with held-out labeled cases. Do not claim the library automatically learns from streaming measurements on the device.

### ESP-DSP and S3 acceleration

- [ESP-DSP official examples](https://github.com/espressif/esp-dsp/tree/master/examples): basic math, IIR, FIR, FFT and Kalman.
- [ESP-NN performance and supported chips](https://github.com/espressif/esp-nn): optimized s8 kernels for ESP32-S3 vector instructions.
- [ESP-TFLite-Micro](https://github.com/espressif/esp-tflite-micro) has integrated ESP-NN examples. Its published S3 person-detection speedup uses internal RAM, not a PSRAM-constrained GrowClip profile.
- [ESP-DL](https://github.com/espressif/esp-dl) supports quantized model deployment and its own `.espdl` tooling. Treat its broad model demos as **capability references**, not evidence that an anomaly model will fit our resource budget.

## What *not* to copy

- Entire climate/growbox/Nodeflow control, physical output, display or storage stack.
- Unreviewed generated weights or training data from unrelated tasks.
- Generic large TFLM/Edge Impulse binaries without demonstrated advantage.
- Any claimed memory or latency number from another board as though measured under our Wi-Fi, storage, display, TLS and firmware workload.
- Proprietary or restrictive pre-trained weights merely because the runtime library is open source.

## Suggested first evaluation (not an approved sprint)

1. Use a small, **portable time-series sample contract**: numeric value, monotonic timestamp, validity/freshness and source identifier; no hardware polling inside the detector.
2. Extract or reimplement a very small **BloomML-inspired bounded trend** baseline (slope + EWMA or robust deviation) and test missing/stale/constant/spike/drift patterns.
3. Evaluate `emlearn` anomaly scoring against the same labeled replay traces; compare with simple baselines, not just an ML-only score.
4. If numerics dominate, try an isolated **ESP-DSP** implementation. If a neural model actually wins, compare **TinyMaix** and later **ESP-NN/TFLM**.
5. Verify on ESP32-S3 with PSRAM: internal DRAM delta, largest contiguous internal block, PSRAM peak, Flash footprint, inference time, scheduling impact, boot/stop and cache-disabled safety. Also test a constrained 2 MB PSRAM profile if it is a target.
6. Provide outputs to a hypothetical application adapter as **read-only state**. No direct actuation and no changes to GrowClip until a candidate is qualified.

## Decision rules

- Pick by **value versus a trivial deterministic baseline**, not model novelty.
- Prefer a **minimal integration** with a stable license and tests over wholesale copying.
- Track total DRAM, including RTOS worker stack/TCB, global scratch and allocator overhead. External PSRAM is not interchangeable with internal memory in all execution contexts.
- Retain a clear `unavailable` state if data/model/compute are not trustworthy.
- Record adopted upstream revision and run a compact license review at extraction time.

**Current result:** library inventory and code-level donor references only. No models trained, firmware built/flashed, memory measured, or external library imported.
