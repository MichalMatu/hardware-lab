# ESP32-S3 Edge AI Lab — documentation consistency audit

Date: **2026-11-07**  
Documentation source: `hardware-lab main@78b49fcb5ca4bb459a2cd6710d82a94c3d99dd31` before remediation.  
Scope: project README, docs README, PROJECT_DIRECTION.md and REUSE_RESEARCH.md, cross-checked against the referenced BloomML/GrowClip design and selected upstream source/license material. No firmware or hardware benchmark ran.

## Verdict

**Documentation architecture: coherent after the corrections in this commit. Implementation readiness: NOT YET ESTABLISHED.**

The documents agree on an independent, reusable ESP32-S3 time-series analysis module, an observational/readonly result boundary, cautious GrowClip adapter integration, deliberate DRAM minimization and BloomML as a moving donor rather than an automatic dependency. There are no accepted implementation APIs, model choice or numeric performance claims.

The purpose of this audit is to distinguish **documentary clarity** from **experimental evidence**. Fixes below add explicit guardrails; they do not qualify code or hardware.

## Findings and remediation

| ID | Severity | Finding | Documentary disposition | Residual condition |
| --- | --- | --- | --- | --- |
| D01 | High | Edge Impulse repository default BSD license coexists with explicit restrictive subscription language in `dsp/config.hpp` | **Corrected** donor matrix to **license hold / no copy** | Resolve exact per-file license and rights before importing anything |
| D02 | High | Input description lacked reboot/monotonic time, sample gaps, units and quality semantics | **Clarified** sample design invariants | Exact format and data producers OPEN |
| D03 | High | Future Nodeflow output description lacked warm-up, freshness, failure states | **Clarified** result validity and fail-closed contract | Actual Nodeflow conversion/validation OPEN |
| D04 | Medium | DRAM/PSRAM goals lacked baseline method and task/TCB accounting | **Clarified** memory measurement categories and concurrency gate | Physical baseline and limits OPEN |
| D05 | Medium | Offline training data, replay, labels, leakage and false-positive usefulness were underspecified | **Clarified** distinct train/calibration/test and acceptance evidence | Real dataset, target metric and threshold OPEN |
| D06 | Medium | ESP-DSP was presented as part of a default stack although the project prefers simple baselines | **Corrected** to *conditional* mathematical optimization | Benchmark and framework choice OPEN |
| D07 | Medium | Espressif published TFLM internal-memory timing and PSRAM-preferring example could be conflated | **Distinguished** conditions | PSRAM-specific benchmark OPEN |
| D08 | Medium | BloomML moving `main` links were mixed with an exact SHA reference | **Pinned** example source paths to observed donor commit | Recheck donor current head before extraction |
| D09 | Medium | Host library compatibility and portable core versus Arduino/PlatformIO/ESP-IDF boundary not explicit enough | **Clarified** build-system boundary | Target integration POC OPEN |
| D10 | Low | Sensor faults, statistical detections and ML inference were presented as one broad feature family | **Clarified** independent methods behind a possible common result boundary | Final module/API split OPEN |

## External evidence checked

- [BloomML source and design](https://github.com/MichalMatu/bloomml/tree/d5044b712b9f7b56278c31f5770d853836089071): its `ModelRuntime.h` requires serialized use due to generated global scratch; Rule remains authoritative per `docs/ML_DECISION_REPORT.md`.
- [emlearn anomaly documentation](https://emlearn.readthedocs.io/en/stable/anomaly_detection.html): GMM and Mahalanobis/EllipticEnvelope, threshold calibration required.
- [ESP-TFLite-Micro README](https://github.com/espressif/esp-tflite-micro): published performance table states internal memory; [current example code](https://github.com/espressif/esp-tflite-micro/blob/master/examples/person_detection/main/main_functions.cc) prefers a PSRAM arena with internal fallback.
- [Edge Impulse LICENSE](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/LICENSE) and [`dsp/config.hpp`](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/dsp/config.hpp): conflicting/general-versus-file-specific conditions; **do not copy** until resolved.
- [ESP-DL LICENSE](https://github.com/espressif/esp-dl/blob/master/LICENSE) and [TinyMaix LICENSE](https://github.com/sipeed/TinyMaix/blob/master/LICENSE): MIT and Apache-2.0 respectively at repository root; file/asset-specific terms may differ.
- [GrowClip Nodeflow extension guide](https://github.com/MichalMatu/growclip/blob/main/docs/nodeflow_engine/architecture/NODEFLOW_EXTENSION_GUIDE.md) and [Data Logger](https://github.com/MichalMatu/growclip/blob/main/docs/main_docs/architecture/DATA_LOGGER.md): numeric provider and fail-closed approach, cached snapshots and microSD archive semantics. Moving main links are integration context, not frozen proof.

## Open decisions (intentionally not resolved by documentation)

1. **Initial use case and measurable user benefit:** which sensor/phenomenon and what improvement over Compare/Hysteresis/slope is required?
2. **Hardware target and resource budget:** 2 MB vs 16 MB PSRAM, current DRAM/Flash headroom under expected network/storage load, maximum acceptable task/heap regression.
3. **Acquisition:** exact sample producers, sampling interval, units, source health/age, history/rollup handling, session identity after reboot.
4. **Training/evaluation:** access to representative consented data, labeling strategy, scenario and drift coverage, hold-out splits, detection-delay and false-positive targets.
5. **Runtime:** library/framework selection, scratch ownership, task scheduling choice, behavior during Flash-cache-disabled operations, worst-case latency and stop/restart lifecycle.
6. **Adoption:** license acceptance for each donor component and artifact; commit pin, provenance ledger and tests.
7. **Integration:** separate, later GrowClip Nodeflow type/contract/UI work and safety qualification. Neither device actuation nor automatic donor synchronization is authorized.

## Consistency and acceptance rules

- This is a **research scaffold**, not a running product, tested ML system or approved roadmap.
- Anomaly score is not a physical truth, and model confidence is not implicitly calibrated.
- No direct sensor polling/storage/network in the generic analysis core; no output actuation in donor/AI code.
- No live module may silently substitute invalid, warm-up or stale data with a valid trigger.
- Benchmark every chosen solution against a simple deterministic baseline, then against the **same** data on host and ESP32-S3.
- Do not claim zero DRAM or publish a numerical RAM/latency target before source-qualified measurements.
- Preserve each donor's license, provenance and independence; BloomML remains actively developed.

**Audit closeout:** documentation discrepancies addressed in this docs-only change. **Hardware, licensing resolution, model quality, integration, and implementation remain OPEN**, not PASS.
