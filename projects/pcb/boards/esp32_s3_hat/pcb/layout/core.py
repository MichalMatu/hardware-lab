import pcbnew

def mm(value: float) -> int:
    return pcbnew.FromMM(value)

def _find_pad(footprint, number: int):
    target = str(number)
    for pad in footprint.Pads():
        if pad.GetNumber() == target:
            return pad
    raise RuntimeError(f"Pad {target} not found on {footprint.GetReference()}")

def _footprint_by_ref(board, ref: str):
    fp = board.FindFootprintByReference(ref)
    if fp is None:
        return None
    if hasattr(fp, "SetPosition"):
        return fp
    return pcbnew.Cast_to_FOOTPRINT(fp)

def _point_mm(x_mm: float, y_mm: float) -> pcbnew.VECTOR2I:
    return pcbnew.VECTOR2I(mm(x_mm), mm(y_mm))
