# Pen holder CAD

This directory is the canonical home for the current Kobra 2 Neo pen-holder mechanical design.

## Source of truth

Keep the editable Fusion 360 source here when it is exported/provided:

- `kobra2-neo-pen-holder.f3d` — canonical editable design source.

Recommended derived artifacts:

- `exports/kobra2-neo-pen-holder.step` — neutral CAD exchange export;
- `exports/kobra2-neo-pen-holder.stl` — printable mesh for the exact validated revision;
- optional `.3mf` if slicer/project metadata is useful;
- `images/` — renders/photos showing installation orientation and critical clearances.

Do not keep STL/3MF as the only design artifact.

## Calibration relationship

The current machine calibration is tied to the presently installed holder and pen:

- pen-up Z=6.12;
- pen-down Z=3.12;
- hard pen-tip envelope X=3..223, Y=36..230 mm.

Any CAD revision that changes pen length, clamp position, carriage offset, compliance or reference-sensor geometry invalidates the affected calibration until it is physically rechecked.

## Current mechanical note

The existing holder works for plotting but does not currently use the rear `z_max` button as an automatic pen-tool reference. A future revision may change that, but the working baseline should remain reproducible until the replacement has been validated.
