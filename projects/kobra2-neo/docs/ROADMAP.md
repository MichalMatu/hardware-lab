# Roadmap

## Phase 0 — Bootstrap and verified baseline

- [x] Working USB serial path identified.
- [x] Stock Marlin communication verified.
- [x] Bounded X/Y/Z motion verified.
- [x] Original head/probe removed.
- [x] Project isolated under `hardware-lab/projects/kobra2-neo`.

## Phase 1 — Bare carriage and common interface

- [ ] Measure the bare carriage: mounting holes, spacing, offsets, keep-out zones and accessible fasteners.
- [ ] Capture photos/dimensions and define the carriage coordinate datum.
- [ ] Design an editable common adapter plate/tool interface.
- [ ] Validate mass, rigidity, clearance and cable/strain-relief strategy.

## Phase 2 — Pen/marker tool

- [ ] Design the first removable pen holder against the common interface.
- [ ] Provide manual Z adjustment/compliance.
- [ ] Establish pen-up / pen-down calibration without relying on the removed stock probe.
- [ ] Dry-run a bounded path above paper, then make the first real plot.

## Phase 3 — Independent Z reference

- [ ] Select a repeatable Z-reference mechanism independent of the stock hotend.
- [ ] Validate safe homing/reference workflow.
- [ ] Only after validation, allow automated Z-dependent workflows.

## Phase 4 — Plotter workflow

- [ ] Define SVG/text/image -> path -> bounded G-code pipeline.
- [ ] Add project-local safety/calibration metadata.
- [ ] Add known-safe sample files and dry-run support.

## Phase 5 — Clay/paste tool

- [ ] Define material delivery architecture and required force/flow.
- [ ] Prefer remote reservoir + light carriage nozzle where practical.
- [ ] Design toolhead, hose/cable management and cleaning workflow.
- [ ] Decide whether a dedicated actuator/controller is required.

## Phase 6 — Firmware decision

Revisit Klipper or other firmware only when a concrete requirement cannot be met cleanly by stock Marlin. Before any flash, identify the exact controller board/MCU and document rollback/recovery.
