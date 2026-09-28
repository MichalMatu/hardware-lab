#!/usr/bin/env python3
"""Dump bounding box dimensions for all footprints to help with placement."""
import sys
import pcbnew

def main():
    board = pcbnew.LoadBoard(sys.argv[1] if len(sys.argv) > 1 else "out/project.kicad_pcb")
    
    print(f"{'Reference':<20} {'X (mm)':>8} {'Y (mm)':>8} {'W (mm)':>8} {'H (mm)':>8}  {'X_min':>8} {'X_max':>8} {'Y_min':>8} {'Y_max':>8}")
    print("-" * 108)
    
    fps = sorted(board.GetFootprints(), key=lambda f: f.GetReference())
    for fp in fps:
        ref = fp.GetReference()
        if not ref:
            continue
        box = fp.GetBoundingBox(False, False)
        cx = pcbnew.ToMM(fp.GetPosition().x)
        cy = pcbnew.ToMM(fp.GetPosition().y)
        w = pcbnew.ToMM(box.GetWidth())
        h = pcbnew.ToMM(box.GetHeight())
        x_min = pcbnew.ToMM(box.GetX())
        x_max = x_min + w
        y_min = pcbnew.ToMM(box.GetY())
        y_max = y_min + h
        print(f"{ref:<20} {cx:>8.2f} {cy:>8.2f} {w:>8.2f} {h:>8.2f}  {x_min:>8.2f} {x_max:>8.2f} {y_min:>8.2f} {y_max:>8.2f}")

if __name__ == "__main__":
    main()
