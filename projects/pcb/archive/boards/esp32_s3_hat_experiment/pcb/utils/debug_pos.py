import sys
import pcbnew

def main():
    board = pcbnew.LoadBoard(sys.argv[1])
    for fp in board.GetFootprints():
        ref = fp.GetReference()
        if ref in ["U_PMIC", "C_DCDC1", "L_PMIC_SW", "Q_CHG", "J_HOST_L"]:
            pos = fp.GetPosition()
            bb = fp.GetBoundingBox(False, False)
            print(f"{ref}: POS=({pcbnew.ToMM(pos.x):.2f}, {pcbnew.ToMM(pos.y):.2f}) "
                  f"BB=(W:{pcbnew.ToMM(bb.GetWidth()):.2f}, H:{pcbnew.ToMM(bb.GetHeight()):.2f}, "
                  f"X:{pcbnew.ToMM(bb.GetX()):.2f}, Y:{pcbnew.ToMM(bb.GetY()):.2f})")

if __name__ == "__main__":
    main()
