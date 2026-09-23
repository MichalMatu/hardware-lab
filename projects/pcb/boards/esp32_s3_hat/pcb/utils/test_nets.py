import pcbnew
import sys

def main():
    board = pcbnew.LoadBoard("out/project.kicad_pcb")
    pmic = board.FindFootprintByReference("U_PMIC")
    
    if not pmic:
        print("PMIC not found")
        sys.exit(1)
        
    for pad in pmic.Pads():
        net = pad.GetNet()
        if net and net.GetNetCode() > 0:
            print(f"Pad {pad.GetPadName()} connected to net {net.GetNetname()}")

if __name__ == "__main__":
    main()
