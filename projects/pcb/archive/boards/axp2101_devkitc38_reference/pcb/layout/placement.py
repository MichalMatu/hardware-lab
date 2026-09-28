import pcbnew
from layout.constants import PLACEMENTS, BOTTOM_SIDE_REFS, PIN_COUNT, SILKLESS_REFS
from layout.core import mm, _find_pad, _footprint_by_ref

def place_fixed_components(board):
    print("Placing host sockets...")
    for ref, (x, y, rot) in PLACEMENTS.items():
        fp = _footprint_by_ref(board, ref)
        if fp is None:
            print(f"Warning: Footprint {ref} not found on board.")
            continue
        fp.SetPosition(pcbnew.VECTOR2I(mm(x), mm(y)))
        should_be_flipped = ref in BOTTOM_SIDE_REFS
        if fp.IsFlipped() != should_be_flipped:
            fp.Flip(fp.GetPosition(), False)
        if hasattr(fp, "SetOrientationDegrees"):
            fp.SetOrientationDegrees(rot)
        else:
            fp.SetOrientation(int(rot * 10))

        if ref in {"J_HOST_L", "J_HOST_R"}:
            pad_1 = _find_pad(fp, 1)
            pad_n = _find_pad(fp, PIN_COUNT)
            if pad_1.GetPosition().y > pad_n.GetPosition().y:
                if hasattr(fp, "SetOrientationDegrees"):
                    fp.SetOrientationDegrees((rot + 180) % 360)
                else:
                    fp.SetOrientation(int((rot + 180) * 10))

def tidy_annotations(board):
    for fp in board.GetFootprints():
        ref = fp.Reference()
        value = fp.Value()
        if ref:
            ref.SetVisible(False)
        if value:
            value.SetVisible(False)
        if fp.GetReference() in SILKLESS_REFS:
            for item in fp.GraphicalItems():
                layer = item.GetLayer()
                if layer == pcbnew.F_SilkS:
                    item.SetLayer(pcbnew.F_Fab)
                elif layer == pcbnew.B_SilkS:
                    item.SetLayer(pcbnew.B_Fab)
