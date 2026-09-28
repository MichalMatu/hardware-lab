from __future__ import annotations

import pcbnew


def mm(value: float) -> int:
    """Convert millimetres to KiCad internal units."""
    return pcbnew.FromMM(value)


def point_mm(x_mm: float, y_mm: float) -> pcbnew.VECTOR2I:
    """Create a KiCad point from millimetre coordinates."""
    return pcbnew.VECTOR2I(mm(x_mm), mm(y_mm))


def find_pad(footprint, number: int | str):
    """Return a pad by its physical pad number."""
    target = str(number)
    for pad in footprint.Pads():
        if pad.GetNumber() == target:
            return pad
    raise RuntimeError(f"Pad {target} not found on {footprint.GetReference()}")


def footprint_by_ref(board, ref: str):
    """Return a footprint by reference, normalizing SWIG wrapper types."""
    footprint = board.FindFootprintByReference(ref)
    if footprint is None:
        return None
    if hasattr(footprint, "SetPosition"):
        return footprint
    return pcbnew.Cast_to_FOOTPRINT(footprint)
