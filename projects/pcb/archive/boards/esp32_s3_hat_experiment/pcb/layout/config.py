import json
from pathlib import Path
import pcbnew
from layout.constants import (
    FOUR_LAYER_COPPER_LAYERS, LAYER_STRATEGY, EXACT_SIZE_EDGE_CLEARANCE_MM
)

def configure_four_layer_board(board):
    print("Configuring 4-layer stackup...")
    board.SetCopperLayerCount(len(FOUR_LAYER_COPPER_LAYERS))

    design_settings = board.GetDesignSettings()
    if hasattr(design_settings, "SetCopperLayerCount"):
        design_settings.SetCopperLayerCount(len(FOUR_LAYER_COPPER_LAYERS))

    enabled_layers = board.GetEnabledLayers()
    visible_layers = board.GetVisibleLayers()
    for layer in FOUR_LAYER_COPPER_LAYERS:
        enabled_layers.AddLayer(layer)
        visible_layers.AddLayer(layer)
        board.SetLayerName(layer, board.GetStandardLayerName(layer))

    board.SetEnabledLayers(enabled_layers)
    board.SetVisibleLayers(visible_layers)
    if hasattr(design_settings, "SetEnabledLayers"):
        design_settings.SetEnabledLayers(enabled_layers)

    for layer_name, purpose in LAYER_STRATEGY:
        print(f"  {layer_name}: {purpose}")

def clear_routing(board):
    print("Clearing tracks, vias, zones, and generated drawings...")
    for item in list(board.GetTracks()):
        board.Remove(item)

    for zone in list(board.Zones()):
        board.Remove(zone)

    clear_layers = {
        pcbnew.Edge_Cuts,
        pcbnew.Dwgs_User,
        pcbnew.F_SilkS,
        pcbnew.F_Fab,
        pcbnew.User_1,
    }
    try:
        drawings = list(board.GetDrawings())
    except Exception:
        try:
            drawings_obj = board.Drawings()
            drawings = [drawings_obj[index] for index in range(len(drawings_obj))]
        except Exception:
            drawings = []

    for drawing in drawings:
        if drawing.GetLayer() in clear_layers:
            board.Remove(drawing)

def update_project_rules(board_path: str) -> None:
    project_path = Path(board_path).with_suffix(".kicad_pro")
    if not project_path.exists():
        return

    project_data = json.loads(project_path.read_text())
    design_settings = project_data.setdefault("board", {}).setdefault("design_settings", {})
    rules = design_settings.setdefault("rules", {})
    rules["min_copper_edge_clearance"] = EXACT_SIZE_EDGE_CLEARANCE_MM
    project_path.write_text(json.dumps(project_data, indent=2) + "\n")
