# Pen plotter calibration

## Calibration scope

These values describe the current physical combination of:

- Anycubic Kobra 2 Neo;
- current pen/marker;
- current printed holder/adapter;
- current paper placement;
- current machine mechanics.

Changing any of those may invalidate the calibration.

## Pen-tip work envelope — 2026-09-28

The real pen tip was manually positioned at the four paper/work-area corners and read with `M114` after a valid machine reference had been established.

Measured corners at pen-up Z=6.12:

- upper-left: X=3.00, Y=230.00;
- upper-right: X=223.00, Y=230.00;
- lower-right: X=223.00, Y=36.00;
- lower-left: X=3.00, Y=36.00.

Canonical pen-tip envelope:

- `X_min = 3.00`
- `X_max = 223.00`
- `Y_min = 36.00`
- `Y_max = 230.00`
- width = 220 mm
- height = 194 mm
- geometric center = X=113.00, Y=133.00

The earlier manually chosen point X=114.00, Y=149.00 was a visually selected build-plate/paper reference point, not the geometric center of this measured pen-tip envelope.

## Plotting margin

Normal generated artwork uses an additional internal margin of 5 mm inside the measured envelope.

Therefore generated artwork should normally fit inside:

- X=8..218 mm;
- Y=41..225 mm.

The outer measured envelope remains the hard safety boundary for both pen-down and pen-up XY travel during normal automated plotting.

## Z calibration

- pen-up: `Z=6.12`
- pen-down: `Z=2.97`

Pen-up has been physically checked to provide clear travel above the paper. During the 2026-09-28 live session, `G28 Z` reported Z=2.97 while the pen tip was at paper contact. A complete 10 cm artwork was then drawn successfully using pen-down Z=2.97 and pen-up Z=6.12, so 2.97 is the current validated drawing value.

Do not treat these as machine-independent constants. Revalidate after changing pen length, holder geometry, paper thickness/position, carriage/tool mechanics or any Z-reference geometry.

## Orientation

The currently validated plotter profile uses:

- no XY swap;
- no X flip;
- Y flip enabled.

Orientation is a source-to-machine mapping choice and should be verified in dry-run/preview before live execution when the input pipeline changes.
