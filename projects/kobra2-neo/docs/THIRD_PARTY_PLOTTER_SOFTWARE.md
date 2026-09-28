# Third-party plotter software candidates

Status: evaluation backlog. None of the tools below is considered validated for the Kobra 2 Neo pen profile until its generated G-code has been checked against the project-local safety policy and current calibration.

Current Kobra pen profile remains canonical in `config/kobra2_neo_pen.toml`. At the time of this note the important values are pen-up Z=6.12 mm, pen-down Z=2.97 mm, normal drawing envelope X=8..218 mm / Y=41..225 mm, travel feed 3000 mm/min, draw feed 1200 mm/min and Z feed 180 mm/min.

## 1. GCodeScribe

Repository: https://github.com/Friedjof/gcodescribe

Why evaluate it:
- browser-based pen-plotter studio;
- imports PDF, SVG, raster images and Office documents;
- uses vpype and can trace raster/scanned content;
- interactive layout and G-code preview;
- calibration profiles for plot area, offsets, margins, feed rates and pen-up / pen-down Z;
- supports OctoPrint and direct USB-serial Marlin backends;
- has explicit safety checks and profile/job fingerprints.

Kobra test goals:
- create a Kobra 2 Neo calibration profile matching our current values;
- verify generated G-code contains no heaters, extrusion, unexpected homing or out-of-bounds moves;
- test JPG/PNG line-art, SVG, text and PDF conversion;
- compare its direct serial path with our existing `kobra-plot` + Pronterface workflow.

## 2. OmniPlot (`rsp-pen-plotter`)

Repository: https://github.com/glloq/rsp-pen-plotter

Why evaluate it:
- universal workflow for bitmap, SVG, PDF, CAD, Office/text and G-code inputs;
- many raster-to-line algorithms including potrace, stippling, hatching and halftone;
- machine profiles are YAML-driven;
- preview, bounds checks, time estimates and toolpath optimization;
- simulator and manual control UI;
- workstation/dev mode exists in addition to the Raspberry Pi appliance path.

Caveat: the appliance installer is written for Raspberry Pi OS / Debian / Ubuntu. On macOS use the workstation/dev path rather than the apt/systemd installer.

Kobra test goals:
- define a Marlin/Kobra machine profile without changing the canonical calibration source in this repository;
- validate Z semantics for pen up/down;
- verify its generated G-code against `kobra-plot` safety rules before live use;
- compare photo conversion modes and path optimization quality.

## 3. Plottter

Repository: https://github.com/pywkt/plottter

Why evaluate it:
- native desktop-oriented Python application for macOS/Linux/Windows;
- strong raster-to-vector toolset: Canny edges, hatching, flow fields, stippling, contour lines, XDoG/FDoG, hedcut, scanline halftone, spiral portraits, sketch and more;
- text rendering and generative-art tools;
- exports SVG, HPGL and G-code;
- path optimization, simplification, merge and duplicate removal;
- useful as an art/toolpath generator even if final Kobra G-code is produced by our own safety layer.

Kobra test goals:
- first use Plottter primarily as `image -> optimized SVG`;
- feed resulting SVG through `kobra-plot` so current Kobra safety/calibration remains authoritative;
- only evaluate Plottter's direct G-code output after its machine/export settings are audited.

## Recommended evaluation order

1. Plottter for JPG/PNG -> SVG because it is the least coupled to printer control and offers the richest image algorithms.
2. GCodeScribe for an all-in-one conversion/calibration/Marlin workflow.
3. OmniPlot for its universal converter set and machine-profile architecture.

## Safety boundary

Third-party installation or successful preview does not make a generated job safe for this printer. Before any live Kobra execution, retain the existing project rules: inspect the complete job, enforce current bounds and pen Z values, reject heater/extrusion commands, finish pen-up and use an explicit live action.
