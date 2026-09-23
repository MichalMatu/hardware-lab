#!/bin/sh
set -eu

BOARD_DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "$BOARD_DIR/../.." && pwd)"

KICAD_PYTHON="${KICAD_PYTHON:-/Applications/KiCad/KiCad.app/Contents/Frameworks/Python.framework/Versions/3.9/bin/python3}"
EXTRA_PYTHONPATH="${EXTRA_PYTHONPATH:-/Library/Frameworks/Python.framework/Versions/3.12/lib/python3.12/site-packages}"
BRIDGED_PYTHONPATH="$REPO_ROOT/templates/skidl_reference/src:$EXTRA_PYTHONPATH"

PYTHONPATH="$BRIDGED_PYTHONPATH${PYTHONPATH:+:$PYTHONPATH}" \
    "$KICAD_PYTHON" "$BOARD_DIR/src/main.py"

PYTHONPATH="$BRIDGED_PYTHONPATH${PYTHONPATH:+:$PYTHONPATH}" \
    "$KICAD_PYTHON" "$BOARD_DIR/pcb/layout_automation.py" "$BOARD_DIR/out/project.kicad_pcb"
