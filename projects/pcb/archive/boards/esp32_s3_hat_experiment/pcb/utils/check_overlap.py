#!/usr/import/env python3
import sys
import pcbnew
from itertools import combinations

def main():
    if len(sys.argv) < 2:
        print("Usage: check_overlap.py <board.kicad_pcb>")
        sys.exit(1)

    board_path = sys.argv[1]
    board = pcbnew.LoadBoard(board_path)

    footprints = list(board.GetFootprints())
    overlaps = []

    # Filter out footprints that are allowed to overlap or don't matter as much for basic placement checking
    # (e.g. host connectors might overlap board edges or drawings)
    
    for fp1, fp2 in combinations(footprints, 2):
        ref1 = fp1.GetReference()
        ref2 = fp2.GetReference()
        
        # skip unreferenced
        if not ref1 or not ref2:
            continue

        # Ignore overlaps between host headers if they happen (though they shouldn't)
        if ref1.startswith("J_HOST") and ref2.startswith("J_HOST"):
            continue

        box1 = fp1.GetBoundingBox(False, False)
        box2 = fp2.GetBoundingBox(False, False)

        # check intersection
        if box1.Intersects(box2):
            # calculate intersection area or just report
            # Inflate a bit to see if they are too close? 
            # Intersects() checks if they touch or overlap. 
            # Let's get the overlap rectangle.
            intersect_box = box1.Intersect(box2)
            w = intersect_box.GetWidth()
            h = intersect_box.GetHeight()
            
            # If overlap is extremely small (e.g., < 0.05 mm), it might just be touching courtyards
            if w > pcbnew.FromMM(0.05) and h > pcbnew.FromMM(0.05):
                overlaps.append((ref1, ref2, pcbnew.ToMM(w), pcbnew.ToMM(h)))

    if not overlaps:
        print("SUCCESS: 0 overlaps found!")
        sys.exit(0)
    else:
        print(f"FAILED: Found {len(overlaps)} overlaps:")
        for r1, r2, w, h in overlaps:
            print(f"  Overlap between {r1} and {r2} (Area: {w:.2f}mm x {h:.2f}mm)")
        sys.exit(1)

if __name__ == "__main__":
    main()
