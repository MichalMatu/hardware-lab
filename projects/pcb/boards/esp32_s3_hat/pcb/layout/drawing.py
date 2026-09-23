from __future__ import annotations
import pcbnew
from layout.constants import (
    BOARD_LEFT_MM, BOARD_TOP_MM, BOARD_RIGHT_MM, BOARD_BOTTOM_MM,
    LEFT_ROW_X_MM, RIGHT_ROW_X_MM, TOP_PIN_Y_MM,
    PIN_COUNT, PIN_PITCH_MM, EDGE_WIDTH_MM,
    BOARD_CENTER_X_MM, LEFT_PIN_LABELS, RIGHT_PIN_LABELS
)
from layout.core import mm, _point_mm, _find_pad, _footprint_by_ref

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
    power_left = LEFT_ROW_X_MM - 0.5
    power_right = RIGHT_ROW_X_MM + 0.5
    power_top = TOP_PIN_Y_MM + (PIN_COUNT - 1) * PIN_PITCH_MM + 4.0
    power_bottom = BOARD_BOTTOM_MM - 4.5

    bat_left = BOARD_LEFT_MM + 1.8
    bat_right = LEFT_ROW_X_MM - 1.8
    bat_top = power_top
    bat_bottom = power_bottom

    service_left = RIGHT_ROW_X_MM + 1.8
    service_right = BOARD_RIGHT_MM - 1.8
    service_top = power_top
    service_bottom = power_bottom

    _add_rect(board, bat_left, bat_top, bat_right, bat_bottom, pcbnew.Dwgs_User)
    _add_rect(board, power_left, power_top, power_right, power_bottom, pcbnew.Dwgs_User)
    _add_rect(board, service_left, service_top, service_right, service_bottom, pcbnew.Dwgs_User)

    _add_text(
        board,
        "BAT / AON",
        (bat_left + bat_right) / 2.0,
        bat_top + 2.2,
        pcbnew.Dwgs_User,
        size_mm=0.9,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )
    _add_text(
        board,
        "AXP2101 POWER CORE",
        (power_left + power_right) / 2.0,
        power_top + 2.2,
        pcbnew.Dwgs_User,
        size_mm=0.9,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )
    _add_text(
        board,
        "STATUS / TEST",
        (service_left + service_right) / 2.0,
        service_top + 2.2,
        pcbnew.Dwgs_User,
        size_mm=0.9,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )

    _add_text(
        board,
        "VBUS from ESP_5V",
        BOARD_CENTER_X_MM,
        power_bottom - 3.4,
        pcbnew.Dwgs_User,
        size_mm=0.8,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )
    _add_text(
        board,
        "Keep switching power below host board",
        BOARD_CENTER_X_MM,
        power_bottom - 1.8,
        pcbnew.Dwgs_User,
        size_mm=0.75,
        thickness_mm=0.1,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )
    _add_text(
        board,
        "AUTO HOST 3V3 SW",
        16.2,
        44.8,
        pcbnew.Dwgs_User,
        size_mm=0.75,
        thickness_mm=0.1,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )

    _add_text(
        board,
        "AXP I2C -> IO21 / IO22",
        RIGHT_ROW_X_MM + 7.0,
        TOP_PIN_Y_MM + 6.2,
        pcbnew.Dwgs_User,
        size_mm=0.8,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_LEFT,
    )
    _add_text(
        board,
        "AXP AUX -> IO32 / IO33 / IO25 / IO27",
        LEFT_ROW_X_MM - 7.0,
        TOP_PIN_Y_MM + 16.0,
        pcbnew.Dwgs_User,
        size_mm=0.8,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_RIGHT,
    )
    _add_text(
        board,
        "IRQ=32  PWROK=33",
        LEFT_ROW_X_MM - 7.0,
        TOP_PIN_Y_MM + 18.8,
        pcbnew.Dwgs_User,
        size_mm=0.75,
        thickness_mm=0.1,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_RIGHT,
    )
    _add_text(
        board,
        "CHGLED=25  PWRON_BTN=27",
        LEFT_ROW_X_MM - 7.0,
        TOP_PIN_Y_MM + 21.4,
        pcbnew.Dwgs_User,
        size_mm=0.75,
        thickness_mm=0.1,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_RIGHT,
    )

def draw_pin_labels(board):
    left_fp = _footprint_by_ref(board, "J_HOST_L")
    right_fp = _footprint_by_ref(board, "J_HOST_R")
    if left_fp is None or right_fp is None:
        raise RuntimeError("Host socket footprints were not found on board.")

    _add_text(
        board,
        "ESP32 HOST LEFT / J2",
        LEFT_ROW_X_MM,
        TOP_PIN_Y_MM - 3.7,
        pcbnew.F_SilkS,
        size_mm=0.9,
        thickness_mm=0.12,
        h_justify=pcbnew.GR_TEXT_H_ALIGN_CENTER,
    )
    _add_text(
        board,
        "ESP32 HOST RIGHT / J3",
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
