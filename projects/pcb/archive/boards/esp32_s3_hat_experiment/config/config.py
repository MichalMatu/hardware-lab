import os
from pathlib import Path


def _first_env(*names):
    for name in names:
        value = os.environ.get(name, "")
        if value:
            return value
    return ""


def _infer_footprint_dir(kicad_symbol_dir):
    if not kicad_symbol_dir:
        return ""
    suffix = "/symbols"
    if kicad_symbol_dir.endswith(suffix):
        return kicad_symbol_dir[: -len(suffix)] + "/footprints"
    return ""


def _autodetect_kicad_dirs():
    shared_support_roots = (
        Path("/Applications/KiCad/KiCad.app/Contents/SharedSupport"),
        Path.home() / "Applications/KiCad/KiCad.app/Contents/SharedSupport",
    )
    for root in shared_support_roots:
        symbols = root / "symbols"
        footprints = root / "footprints"
        if symbols.is_dir():
            return str(symbols), str(footprints) if footprints.is_dir() else ""
    return "", ""


def prepare_kicad_env():
    kicad_sym_dir = _first_env(
        "KICAD_SYMBOL_DIR",
        "KICAD9_SYMBOL_DIR",
        "KICAD8_SYMBOL_DIR",
        "KICAD7_SYMBOL_DIR",
        "KICAD6_SYMBOL_DIR",
    )
    autodetected_sym_dir = ""
    autodetected_foot_dir = ""
    if not kicad_sym_dir:
        autodetected_sym_dir, autodetected_foot_dir = _autodetect_kicad_dirs()
        kicad_sym_dir = autodetected_sym_dir
        if kicad_sym_dir:
            os.environ.setdefault("KICAD_SYMBOL_DIR", kicad_sym_dir)

    if not kicad_sym_dir:
        return

    for var in ("KICAD6_SYMBOL_DIR", "KICAD7_SYMBOL_DIR", "KICAD8_SYMBOL_DIR", "KICAD9_SYMBOL_DIR"):
        os.environ.setdefault(var, kicad_sym_dir)

    kicad_foot_dir = _first_env(
        "KICAD_FOOTPRINT_DIR",
        "KICAD9_FOOTPRINT_DIR",
        "KICAD8_FOOTPRINT_DIR",
        "KICAD7_FOOTPRINT_DIR",
        "KICAD6_FOOTPRINT_DIR",
    )
    if not kicad_foot_dir:
        if autodetected_foot_dir:
            kicad_foot_dir = autodetected_foot_dir
        else:
            kicad_foot_dir = _infer_footprint_dir(kicad_sym_dir)
        if kicad_foot_dir:
            os.environ.setdefault("KICAD_FOOTPRINT_DIR", kicad_foot_dir)

    if kicad_foot_dir:
        for var in ("KICAD6_FOOTPRINT_DIR", "KICAD7_FOOTPRINT_DIR", "KICAD8_FOOTPRINT_DIR", "KICAD9_FOOTPRINT_DIR"):
            os.environ.setdefault(var, kicad_foot_dir)


def setup_skidl():
    prepare_kicad_env()

    kicad_sym_dir = os.environ.get("KICAD_SYMBOL_DIR", "")
    if not kicad_sym_dir:
        raise RuntimeError("Set KICAD_SYMBOL_DIR or edit config/config.py")

    from skidl import KICAD, set_default_tool, lib_search_paths

    set_default_tool(KICAD)
    if kicad_sym_dir not in lib_search_paths[KICAD]:
        lib_search_paths[KICAD].append(kicad_sym_dir)
