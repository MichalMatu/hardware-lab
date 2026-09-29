# Safety contract

This contract applies to the current dedicated pen-plotter hardware.

## Hardware assumptions

The original printhead/hotend is intentionally removed. There is no hotend heater cartridge, no hotend thermistor and no printhead fan hardware. The active tool is a pen/marker. The cylindrical magnetic/proximity sensor is the verified `z_min` used for Z homing.

## Non-negotiable rules

1. `config/kobra2_neo_pen.toml` is the executable source of truth for current bounds, Z values, feeds and orientation. If code/docs/profile disagree, stop before physical execution.
2. Current hard pen-tip envelope is `X=3..223`, `Y=36..230` mm. Normal generated artwork uses the 5 mm internal envelope `X=8..218`, `Y=41..225` mm.
3. Current pen-down is `Z=2.97`; current pen-up is `Z=4.97`; current Z feed is `360 mm/min`. Old `Z6.12 F180` values are historical.
4. Current XY feeds are travel `6000` and draw `2400` mm/min.
5. Changing the pen, holder, paper thickness/position, carriage mechanics or Z-reference geometry invalidates dependent calibration until revalidated.
6. Homing belongs to the approved PRINT preamble. Prepared artwork must not contain `G28`.
7. Prepared artwork must not contain heater commands, extrusion, unexpected relative mode, out-of-envelope XY motion, unexpected Z values or unexpected feeds.
8. Never enable nozzle heating from G-code or the printer UI in the current headless pen configuration.
9. Idle hotend `T:0.00` is expected because the thermistor is absent and must not by itself block pen plotting.
10. A firmware-emitted `MINTEMP`, `MAXTEMP`, `Printer halted`, `kill() called` or equivalent during an active transaction is terminal. Do not ignore an actual firmware kill because the thermistor is intentionally absent.
11. After a firmware halt, do not claim final pen-up unless acknowledged motion proves it. Treat final pen state as unknown until physically inspected/reset.
12. The printer must be identified with `M115` before deliberate plot motion. CH340 VID/PID or a remembered `/dev/cu.usbserial-*` path is not sufficient identity.
13. Never open competing serial sessions against the printer while a live task is active.
14. Before `G28 X Y`, clear the complete physical travel path, especially the rear Y-bed path and power cable.
15. Use `M400` when confirmed planner completion is part of the transaction contract.
16. A prepared job is not permission to run it. Review the actual preview/report/G-code and current physical setup before one explicit approval of the full transaction.
17. PRINT must consume an already prepared immutable job. It must not edit code, regenerate artwork, change profile values, commit or push.
18. Physical status claims require evidence: do not say drawing started before `DRAWING_STARTED`, and do not say completed before terminal pen-up acknowledgement.
19. If a stage fails, stop that stage. One corrective retry is allowed only after the exact root cause is identified. A second failure ends the attempt; do not create a multi-hour chain of speculative retries.

## Transaction boundary

One approval may cover exactly one reviewed transaction:

```text
identity -> G28 X Y -> G28 Z -> pen up -> immutable artwork stream -> M400 -> final pen up -> terminal acknowledgement
```

Any error/unsafe condition terminates that authorization.