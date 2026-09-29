# Pen plotter calibration

`config/kobra2_neo_pen.toml` is the executable calibration source of truth. This document explains the current values and when they become invalid.

## Calibration scope

The values apply to the current combination of:

- Anycubic Kobra 2 Neo mechanics;
- current pen/marker;
- current printed holder/adapter;
- current paper thickness/placement;
- current Z-reference geometry.

Changing any of those may require recalibration.

## Pen-tip work envelope

Measured physical envelope:

```text
X_min = 3.00 mm
X_max = 223.00 mm
Y_min = 36.00 mm
Y_max = 230.00 mm
width = 220 mm
height = 194 mm
```

Normal generated artwork keeps a 5 mm internal margin:

```text
X=8..218 mm
Y=41..225 mm
```

The outer envelope is the hard plotting boundary; the inner envelope is the normal generated-artwork boundary.

## Z calibration — current baseline

```text
pen down = Z2.97
pen up   = Z4.97
lift     = 2.00 mm
Z feed   = 360 mm/min
```

`Z2.97` is the paper-contact drawing value for the current setup. `Z4.97` is the tuned travel value: exactly 2.00 mm above pen-down, replacing the older `Z6.12` baseline.

Do not reuse `Z6.12` from historical run logs for new prepared jobs.

Revalidate Z after changing pen length, holder geometry, paper thickness/position, carriage/tool mechanics or Z-sensor geometry.

## XY feeds — current baseline

```text
travel feed = 6000 mm/min
draw feed   = 2400 mm/min
```

These replace the older `3000 / 1200` baseline after the successful 2026-09-29 map plot and subsequent requested tuning. Treat further speed changes as a separate calibration/code-maintenance change, not an artwork-preparation detail.

## Orientation

Current mapping:

- no XY swap;
- no X flip;
- Y flip enabled.

Verify orientation in `preview.svg` whenever the source pipeline changes.

## Calibration authority

If this document and `config/kobra2_neo_pen.toml` disagree, stop before PRINT and reconcile them. Do not silently choose whichever value is more convenient.