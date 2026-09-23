#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path

import pcbnew


def mm(value: float) -> int:
    return pcbnew.FromMM(value)


BOARD_LEFT_MM = 0.0
BOARD_TOP_MM = 0.0
BOARD_RIGHT_MM = 27.94
BOARD_BOTTOM_MM = 55.88
ROW_SPACING_MM = 25.40
BOARD_CENTER_X_MM = (BOARD_LEFT_MM + BOARD_RIGHT_MM) / 2.0
LEFT_ROW_X_MM = BOARD_CENTER_X_MM - (ROW_SPACING_MM / 2.0)
RIGHT_ROW_X_MM = BOARD_CENTER_X_MM + (ROW_SPACING_MM / 2.0)
TOP_PIN_Y_MM = 1.27
PIN_PITCH_MM = 2.54
PIN_COUNT = 22
EDGE_WIDTH_MM = 0.1
EXACT_SIZE_EDGE_CLEARANCE_MM = 0.4

PLACEMENTS = {
    # ── Host sockets (F.Cu, fixed by spec) ───────────────────────────────
    "J_HOST_L": (LEFT_ROW_X_MM, TOP_PIN_Y_MM, 0),
    "J_HOST_R": (RIGHT_ROW_X_MM, TOP_PIN_Y_MM, 0),

    # ── PMIC core (B.Cu) - Shifted down by ~5mm for S3 ─────────────────
    "U_PMIC": (13.97, 21.5, 180),

    # LEFT of PMIC
    "L_DCDC1":     (8.5, 21.5, -90),
    "C_DCDC1":     (8.5, 26.0, 0),
    "C_PMIC_VIN1": (5.5, 20.5, 90),
    "C_PMIC_VIN2": (3.8, 20.5, 90),
    "C_PMIC_VRTC": (5.2, 23.5, 90),

    # RIGHT of PMIC
    "C_PMIC_VREF": (19.0, 22.5, 0),
    "C_PMIC_VIN3": (19.0, 20.5, 0),
    "C_PMIC_VIN4": (19.0, 18.5, 0),

    # ABOVE PMIC
    "C_PMIC_ALDOIN": (13.0, 17.0, 0),
    "C_PMIC_BLDOIN": (16.0, 17.0, 0),

    # BELOW PMIC
    "L_PMIC_SW":    (13.5, 26.5, 180),
    "C_PMIC_BAT":   (8.0, 30.5, 90),
    "C_PMIC_SYSA":  (10.5, 30.5, 90),
    "C_PMIC_SYSB":  (13.5, 30.5, 90),
    "C_PMIC_VMID":  (15.5, 30.5, 90),
    "C_PMIC_VBUSA": (17.5, 30.5, 90),
    "C_PMIC_VBUSB": (19.5, 30.5, 90),

    # ── I2C pull-ups (B.Cu) ─────────────────────────────────────
    "R_I2C_SCL":  (5.0, 34.0, 0),
    "R_I2C_SDA":  (8.0, 34.0, 0),
    "R_PMIC_IRQ": (11.0, 34.0, 0),

    # ── HOST 3V3 load switch (B.Cu) ───────────────────────────────
    "U_HOST_3V3_SW":  (15.0, 35.5, 0),
    "R_HOST_3V3_OFF": (18.5, 35.5, 90),
    "R_HOST_3V3_ON":  (20.5, 35.5, 90),

    # ── Charge-LED indicator (B.Cu) ────────────
    "R_CHG_G":   (4.8, 36.5, 0),
    "R_CHG_PD":  (4.8, 38.0, 0),
    "Q_CHG":     (5.1, 40.5, 180),
    "D_CHG":     (4.8, 43.0, 0),
    "R_LED_CHG": (4.8, 45.0, 0),

    # ── PWRON button cluster (B.Cu) ─────────────────────────────────────
    "R_PWRON":  (15.2, 38.5, 180),
    "C_PWRON":  (15.2, 40.0, 180),
    "SW_PWRON": (10.5, 42.0, 90),

    # ── Battery connector (B.Cu, bottom) - Centered body on board axis ─
    "J_BAT":    (12.97, 51.5, 0),

    # ── Test pads (B.Cu, edges) ─────────────────────────────────────────
    "TP_GND":   (4.8, 50.3, 0),
    "TP_TS":    (4.8, 52.7, 0),
    "TP_VSYS":  (23.0, 46.0, 0),
    "TP_VRTC":  (23.0, 48.5, 0),
    "TP_VREF":  (23.0, 51.0, 0),
    "TP_VMID":  (23.0, 53.5, 0),
    "TP_PWRON": (20.5, 53.5, 0),

    # ── Qwiic / STEMMA QT Connector (F.Cu) ──────────────────────────────
    "J_QWIIC":  (13.97, 6.0, 0),
}


BOTTOM_SIDE_REFS = {
    "U_HOST_3V3_SW",
    "R_HOST_3V3_OFF",
    "R_HOST_3V3_ON",
    "R_I2C_SCL",
    "R_I2C_SDA",
    "R_PMIC_IRQ",
    "R_LED_CHG",
    "D_CHG",
    "Q_CHG",
    "R_CHG_G",
    "R_CHG_PD",
    "SW_PWRON",
    "R_PWRON",
    "C_PWRON",
    "U_PMIC",
    "C_PMIC_VBUSA",
    "C_PMIC_VBUSB",
    "C_PMIC_VMID",
    "L_PMIC_SW",
    "C_PMIC_SYSA",
    "C_PMIC_SYSB",
    "C_PMIC_BAT",
    "C_PMIC_VREF",
    "C_PMIC_VRTC",
    "C_PMIC_VIN3",
    "L_DCDC1",
    "C_DCDC1",
    "C_PMIC_VIN4",
    "C_PMIC_ALDOIN",
    "C_PMIC_BLDOIN",
    "C_PMIC_VIN1",
    "C_PMIC_VIN2",
}

SILKLESS_REFS = {
    "J_HOST_L",
    "J_HOST_R",
    "U_PMIC",
}

FOUR_LAYER_COPPER_LAYERS = (
    pcbnew.F_Cu,
    pcbnew.In1_Cu,
    pcbnew.In2_Cu,
    pcbnew.B_Cu,
)

LAYER_STRATEGY = (
    ("F.Cu", "components and critical hat routing"),
    ("In1.Cu", "5V and 3V3 distribution"),
    ("In2.Cu", "solid GND reference plane (adjacent to B.Cu where PMIC and inductors sit)"),
    ("B.Cu", "PMIC, inductors, decouplers and bottom-side signals"),
)

LEFT_PIN_LABELS = [
    ("3V3", "ESP_3V3"),
    ("GND", "GND"),
    ("EN", "ESP_EN"),
    ("SDA", "ESP_GPIO21_SDA"),
    ("SCL", "ESP_GPIO22_SCL"),
    ("IRQ", "ESP_GPIO32"),
    ("PWROK", "ESP_GPIO33"),
    ("CHGLED", "ESP_GPIO25"),
    ("PWRON", "ESP_GPIO27"),
    ("L10", "ESP_GPIO_L10"),
    ("L11", "ESP_GPIO_L11"),
    ("L12", "ESP_GPIO_L12"),
    ("L13", "ESP_GPIO_L13"),
    ("L14", "ESP_GPIO_L14"),
    ("L15", "ESP_GPIO_L15"),
    ("L16", "ESP_GPIO_L16"),
    ("L17", "ESP_GPIO_L17"),
    ("L18", "ESP_GPIO_L18"),
    ("L19", "ESP_GPIO_L19"),
    ("L20", "ESP_GPIO_L20"),
    ("L21", "ESP_GPIO_L21"),
    ("L22", "ESP_GPIO_L22"),
]

RIGHT_PIN_LABELS = [
    ("5V", "ESP_5V"),
    ("GND", "GND"),
    ("R3", "ESP_GPIO_R3"),
    ("R4", "ESP_GPIO_R4"),
    ("R5", "ESP_GPIO_R5"),
    ("R6", "ESP_GPIO_R6"),
    ("R7", "ESP_GPIO_R7"),
    ("R8", "ESP_GPIO_R8"),
    ("R9", "ESP_GPIO_R9"),
    ("R10", "ESP_GPIO_R10"),
    ("R11", "ESP_GPIO_R11"),
    ("R12", "ESP_GPIO_R12"),
    ("R13", "ESP_GPIO_R13"),
    ("R14", "ESP_GPIO_R14"),
    ("R15", "ESP_GPIO_R15"),
    ("R16", "ESP_GPIO_R16"),
    ("R17", "ESP_GPIO_R17"),
    ("R18", "ESP_GPIO_R18"),
    ("R19", "ESP_GPIO_R19"),
    ("R20", "ESP_GPIO_R20"),
    ("R21", "ESP_GPIO_R21"),
    ("R22", "ESP_GPIO_R22"),
]

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


def _add_edge_segment(board, start_mm, end_mm, layer=pcbnew.Edge_Cuts):
    segment = pcbnew.PCB_SHAPE(board)
    segment.SetLayer(layer)
    segment.SetShape(pcbnew.SHAPE_T_SEGMENT)
    segment.SetStart(pcbnew.VECTOR2I(mm(start_mm[0]), mm(start_mm[1])))
    segment.SetEnd(pcbnew.VECTOR2I(mm(end_mm[0]), mm(end_mm[1])))
    segment.SetWidth(mm(EDGE_WIDTH_MM))
    board.Add(segment)


def _add_rect(board, left_mm: float, top_mm: float, right_mm: float, bottom_mm: float, layer: int):
    for start, end in (
        ((left_mm, top_mm), (right_mm, top_mm)),
        ((right_mm, top_mm), (right_mm, bottom_mm)),
        ((right_mm, bottom_mm), (left_mm, bottom_mm)),
        ((left_mm, bottom_mm), (left_mm, top_mm)),
    ):
        _add_edge_segment(board, start, end, layer=layer)


def _add_text(
    board,
    text: str,
    x_mm: float,
    y_mm: float,
    layer: int,
    *,
    size_mm: float,
    thickness_mm: float,
    angle_deg: float = 0.0,
    h_justify: int | None = None,
):
    item = pcbnew.PCB_TEXT(board)
    item.SetText(text)
    item.SetLayer(layer)
    item.SetPosition(pcbnew.VECTOR2I(mm(x_mm), mm(y_mm)))
    item.SetTextSize(pcbnew.VECTOR2I(mm(size_mm), mm(size_mm)))
    item.SetTextThickness(mm(thickness_mm))
    item.SetTextAngleDegrees(angle_deg)
    if h_justify is not None:
        item.SetHorizJustify(h_justify)
    board.Add(item)


def draw_board_outline(board):
    _add_rect(board, BOARD_LEFT_MM, BOARD_TOP_MM, BOARD_RIGHT_MM, BOARD_BOTTOM_MM, pcbnew.Edge_Cuts)

    print(
        f"Board outline: {BOARD_RIGHT_MM - BOARD_LEFT_MM:.1f} mm x "
        f"{BOARD_BOTTOM_MM - BOARD_TOP_MM:.1f} mm"
    )


def draw_host_reference(board):
    host_left = LEFT_ROW_X_MM - 1.27
    host_right = RIGHT_ROW_X_MM + 1.27
    host_top = TOP_PIN_Y_MM - 1.27
    host_bottom = TOP_PIN_Y_MM + (PIN_COUNT - 1) * PIN_PITCH_MM + 1.27

    antenna_left = (host_left + host_right) / 2.0 - 9.0
    antenna_right = antenna_left + 18.0
    antenna_bottom = host_top + 6.04
    _add_rect(board, antenna_left, host_top, antenna_right, antenna_bottom, pcbnew.User_1)


def draw_functional_zones(board):
    _add_text(
        board,
        "VBUS (5V IN)",
        RIGHT_ROW_X_MM - 3.0,
        TOP_PIN_Y_MM + 1.0,
        pcbnew.Dwgs_User,
        size_mm=0.8,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_RIGHT,
    )
    _add_text(
        board,
        "HAT 3V3 OUT",
        LEFT_ROW_X_MM + 3.0,
        TOP_PIN_Y_MM + 1.0,
        pcbnew.Dwgs_User,
        size_mm=0.8,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_LEFT,
    )

    _add_text(
        board,
        "AUTO HOST 3V3 SW",
        16.2,
        33.5,
        pcbnew.Dwgs_User,
        size_mm=0.75,
        thickness_mm=0.1,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )

    _add_text(
        board,
        "AXP I2C (SDA/SCL)",
        LEFT_ROW_X_MM + 3.0,
        TOP_PIN_Y_MM + 3 * PIN_PITCH_MM + 1.27,
        pcbnew.Dwgs_User,
        size_mm=0.8,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_LEFT,
    )
    _add_text(
        board,
        "AXP AUX (IRQ, PWROK, CHGLED, PWRON)",
        LEFT_ROW_X_MM + 3.0,
        TOP_PIN_Y_MM + 5 * PIN_PITCH_MM + 2.54,
        pcbnew.Dwgs_User,
        size_mm=0.75,
        thickness_mm=0.1,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_LEFT,
    )

    _add_text(
        board,
        "Keep switching power below host board",
        BOARD_CENTER_X_MM,
        BOARD_BOTTOM_MM - 1.8,
        pcbnew.Dwgs_User,
        size_mm=0.75,
        thickness_mm=0.1,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )

def draw_pin_labels(board):
    left_fp = _footprint_by_ref(board, "J_HOST_L")
    right_fp = _footprint_by_ref(board, "J_HOST_R")
    if left_fp is None or right_fp is None:
        raise RuntimeError("Host socket footprints were not found on board.")

    _add_text(
        board,
        "ESP32-S3 HOST LEFT / J2",
        LEFT_ROW_X_MM,
        TOP_PIN_Y_MM - 3.7,
        pcbnew.F_SilkS,
        size_mm=0.9,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )
    _add_text(
        board,
        "ESP32-S3 HOST RIGHT / J3",
        RIGHT_ROW_X_MM,
        TOP_PIN_Y_MM - 3.7,
        pcbnew.F_SilkS,
        size_mm=0.9,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )

    for index, (silk, fab) in enumerate(LEFT_PIN_LABELS, start=1):
        pad = _find_pad(left_fp, index)
        y_mm = pcbnew.ToMM(pad.GetPosition().y)
        _add_text(
            board,
            silk,
            LEFT_ROW_X_MM - 2.2,
            y_mm,
            pcbnew.F_SilkS,
            size_mm=0.8,
            thickness_mm=0.12,
            h_justify=pcbnew.GR_TEXT_H_ALIGN_RIGHT,
        )
        _add_text(
            board,
            fab,
            LEFT_ROW_X_MM - 4.0,
            y_mm,
            pcbnew.F_Fab,
            size_mm=0.7,
            thickness_mm=0.1,
            h_justify=pcbnew.GR_TEXT_H_ALIGN_RIGHT,
        )

    for index, (silk, fab) in enumerate(RIGHT_PIN_LABELS, start=1):
        pad = _find_pad(right_fp, index)
        y_mm = pcbnew.ToMM(pad.GetPosition().y)
        _add_text(
            board,
            silk,
            RIGHT_ROW_X_MM + 2.2,
            y_mm,
            pcbnew.F_SilkS,
            size_mm=0.8,
            thickness_mm=0.12,
            h_justify=pcbnew.GR_TEXT_H_ALIGN_LEFT,
        )
        _add_text(
            board,
            fab,
            RIGHT_ROW_X_MM + 4.0,
            y_mm,
            pcbnew.F_Fab,
            size_mm=0.7,
            thickness_mm=0.1,
            h_justify=pcbnew.GR_TEXT_H_ALIGN_LEFT,
        )


def _load_board(board_path: str | None):
    if board_path:
        return pcbnew.LoadBoard(board_path)
    return pcbnew.GetBoard()


def update_project_rules(board_path: str) -> None:
    project_path = Path(board_path).with_suffix(".kicad_pro")
    if not project_path.exists():
        return

    project_data = json.loads(project_path.read_text())
    design_settings = project_data.setdefault("board", {}).setdefault("design_settings", {})
    rules = design_settings.setdefault("rules", {})
    rules["min_copper_edge_clearance"] = EXACT_SIZE_EDGE_CLEARANCE_MM
    project_path.write_text(json.dumps(project_data, indent=2) + "\n")
def add_thermal_vias(board):
    print("Adding thermal vias under PMIC...")
    gnd_net = board.GetNetInfo().GetNetItem("GND")
    if gnd_net is None:
        return
        
    pmic_x, pmic_y = PLACEMENTS["U_PMIC"][0], PLACEMENTS["U_PMIC"][1]
    pitch = 0.5  # 0.5mm spacing
    
    # 5x5 array for maximum thermal performance
    offsets = [-1.0, -0.5, 0.0, 0.5, 1.0]
    
    for dx in offsets:
        for dy in offsets:
            via = pcbnew.PCB_VIA(board)
            board.Add(via)
            via.SetPosition(pcbnew.VECTOR2I(mm(pmic_x + dx), mm(pmic_y + dy)))
            via.SetWidth(mm(0.6))
            via.SetDrill(mm(0.3))
            via.SetNetCode(gnd_net.GetNetCode())


def main():
    parser = argparse.ArgumentParser(description="Apply host header layout to the ESP32-S3 DevKitC HAT PCB.")
    parser.add_argument("board_path", nargs="?", help="Path to .kicad_pcb. If omitted, uses current KiCad board.")
    args = parser.parse_args()

    try:
        board = _load_board(args.board_path)
    except Exception:
        print("Run this script inside KiCad Scripting Console or pass a .kicad_pcb path.")
        return

    configure_four_layer_board(board)
    clear_routing(board)
    place_fixed_components(board)
    tidy_annotations(board)
    draw_board_outline(board)
    draw_host_reference(board)
    draw_functional_zones(board)
    draw_pin_labels(board)
    add_thermal_vias(board)

    if args.board_path:
        pcbnew.SaveBoard(args.board_path, board)
        update_project_rules(args.board_path)
    else:
        pcbnew.Refresh()

    print("Layout automation complete!")


if __name__ == "__main__":
    main()
