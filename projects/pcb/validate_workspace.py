from __future__ import annotations

import sys
from pathlib import Path

import tomllib

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from library.board_profile import load_board_manifest
from library.host_profile import load_host_identity


HOSTS_DIR = ROOT / "hosts"
BOARDS_DIR = ROOT / "boards"
MODULES_DIR = ROOT / "library" / "modules"


def _load_toml(path: Path) -> dict:
    return tomllib.loads(path.read_text(encoding="utf-8"))


def validate_host_dir(host_dir: Path) -> list[str]:
    errors: list[str] = []
    host_toml = host_dir / "host.toml"
    pinout_toml = host_dir / "pinout.toml"
    mechanical_toml = host_dir / "mechanical.toml"

    try:
        identity = load_host_identity(host_toml)
    except Exception as exc:
        return [f"{host_dir.name}: invalid host.toml: {exc}"]

    if not pinout_toml.exists():
        errors.append(f"{identity.id}: missing pinout.toml")
    if not mechanical_toml.exists():
        errors.append(f"{identity.id}: missing mechanical.toml")
    if errors:
        return errors

    try:
        host = _load_toml(host_toml)
        pinout = _load_toml(pinout_toml)
        mechanical = _load_toml(mechanical_toml)

        expected_count = int(host["headers"]["count"])
        expected_pins = int(host["headers"]["pins_per_header"])
        expected_pitch = float(host["headers"]["pitch_mm"])

        header_tables = pinout.get("headers", {})
        if len(header_tables) != expected_count:
            errors.append(
                f"{identity.id}: pinout has {len(header_tables)} headers, expected {expected_count}"
            )
        for header_name, header in header_tables.items():
            pins = header.get("pins", [])
            if len(pins) != expected_pins:
                errors.append(
                    f"{identity.id}: {header_name} has {len(pins)} pins, expected {expected_pins}"
                )

        mech_headers = mechanical["headers"]
        if int(mech_headers["count"]) != expected_count:
            errors.append(f"{identity.id}: mechanical header count differs from host.toml")
        if int(mech_headers["pins_per_header"]) != expected_pins:
            errors.append(f"{identity.id}: mechanical pin count differs from host.toml")
        if float(mech_headers["pitch_mm"]) != expected_pitch:
            errors.append(f"{identity.id}: mechanical pitch differs from host.toml")
        if float(mech_headers["row_spacing_mm"]) <= 0:
            errors.append(f"{identity.id}: invalid header row spacing")

        board = mechanical["board"]
        if float(board["width_mm"]) <= 0 or float(board["height_mm"]) <= 0:
            errors.append(f"{identity.id}: invalid board dimensions")
    except Exception as exc:
        errors.append(f"{identity.id}: profile consistency error: {exc}")

    return errors


def validate_board_dir(board_dir: Path) -> list[str]:
    errors: list[str] = []
    board_toml = board_dir / "board.toml"
    if not board_toml.exists():
        return [f"{board_dir.name}: missing board.toml"]

    try:
        raw = _load_toml(board_toml)
        host_name = str(raw["host"]["profile"]).strip()
        host_dir = HOSTS_DIR / host_name
        host_toml = host_dir / "host.toml"
        if not host_toml.exists():
            return [f"{board_dir.name}: unknown host profile '{host_name}'"]

        host_identity = load_host_identity(host_toml)
        manifest = load_board_manifest(board_toml, host_identity=host_identity)

        for module in manifest.modules:
            module_file = MODULES_DIR / f"{module}.py"
            if not module_file.exists():
                errors.append(
                    f"{manifest.id}: feature module '{module}' has no {module_file.relative_to(ROOT)}"
                )
    except Exception as exc:
        errors.append(f"{board_dir.name}: invalid board manifest: {exc}")

    return errors


def main() -> int:
    errors: list[str] = []

    host_dirs = sorted(
        path for path in HOSTS_DIR.iterdir() if path.is_dir() and (path / "host.toml").exists()
    )
    board_dirs = sorted(
        path for path in BOARDS_DIR.iterdir() if path.is_dir()
    )

    for host_dir in host_dirs:
        errors.extend(validate_host_dir(host_dir))
    for board_dir in board_dirs:
        errors.extend(validate_board_dir(board_dir))

    if errors:
        print("PCB workspace validation FAILED")
        for error in errors:
            print(f"- {error}")
        return 1

    print(
        "PCB workspace validation OK: "
        f"{len(host_dirs)} host profile(s), {len(board_dirs)} active board(s)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
