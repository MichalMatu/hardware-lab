# Pen holder CAD

This directory is the canonical home for the current Kobra 2 Neo pen-holder mechanical design.

## Source of truth

The editable Fusion 360 source is tracked here:

- `kobra2-neo-pen-holder.f3d` — canonical editable design source, imported from the validated `kobra2neo.f3d` supplied on 2026-09-28;
- `SOURCE.sha256` — SHA-256 checksum for the imported source (`bc16edd5d3cf2a7c41ad647d8ffd3002380ad13c0c92f998f67f786b275e85c6`).

Recommended derived artifacts:

- `exports/kobra2-neo-pen-holder.step` — neutral CAD exchange export;
- `exports/kobra2-neo-pen-holder.stl` — printable mesh for the exact validated revision;
- optional `.3mf` if slicer/project metadata is useful;
- `images/` — renders/photos showing installation orientation and critical clearances.

Do not keep STL/3MF as the only design artifact.

## Calibration relationship

The current machine calibration is tied to the presently installed holder and pen:

- pen-up Z=6.12;
- pen-down Z=2.97;
- hard pen-tip envelope X=3..223, Y=36..230 mm.

Any CAD revision that changes pen length, clamp position, carriage offset, compliance or reference-sensor geometry invalidates the affected calibration until it is physically rechecked.

## Current mechanical note

The existing holder works for plotting but does not currently use the rear `z_max` button as an automatic pen-tool reference. A future revision may change that, but the working baseline should remain reproducible until the replacement has been validated.
