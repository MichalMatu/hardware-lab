from __future__ import annotations

import re
import tomllib
from dataclasses import dataclass
from pathlib import Path
from textwrap import dedent

from framework.module_registry import (
    DEFAULT_MODULES,
    MODULE_DEFS,
    RESOURCE_DEFS,
    SUPPORTED_MCU,
    SUPPORTED_POWER,
    ordered_modules_for,
    ordered_resources_for,
    render_call_line,
    validate_modules,
)


@dataclass
class BoardSpec:
    name: str
    description: str
    power: str
    mcu_module: str
    modules: tuple[str, ...]
    netlist: bool
    schematic: bool
    pcb: bool

    @property
    def module_names(self) -> list[str]:
        return list(self.modules)


def _load_modules(data: dict) -> tuple[str, ...]:
    composition = data.get("composition", {})
    raw_modules = composition.get("modules")
    if raw_modules is None:
        return DEFAULT_MODULES

    if not isinstance(raw_modules, list):
        raise ValueError("[composition].modules must be a TOML array.")

    modules = []
    for module in raw_modules:
        if not isinstance(module, str):
            raise ValueError("[composition].modules accepts only strings.")
        normalized = module.strip()
        if normalized:
            modules.append(normalized)
    return tuple(modules)


def load_spec(manifest_path: Path) -> BoardSpec:
    data = tomllib.loads(manifest_path.read_text())
    board = data.get("board", {})
    mcu = data.get("mcu", {})
    outputs = data.get("outputs", {})
    modules = _load_modules(data)

    spec = BoardSpec(
        name=board.get("name", manifest_path.parent.name),
        description=board.get(
            "description",
            "ESP32-S3 board generated from board.toml",
        ),
        power=board.get("power", "usb_5v"),
        mcu_module=mcu.get("module", "esp32-s3-wroom-1"),
        modules=modules,
        netlist=bool(outputs.get("netlist", True)),
        schematic=bool(outputs.get("schematic", False)),
        pcb=bool(outputs.get("pcb", False)),
    )
    validate_spec(spec, manifest_path)
    return spec


def validate_spec(spec: BoardSpec, manifest_path: Path):
    if not re.fullmatch(r"[a-z0-9][a-z0-9_-]*", spec.name):
        raise ValueError(
            f"Invalid board.name in {manifest_path}: use lowercase letters, digits, '_' or '-'."
        )

    if spec.power not in SUPPORTED_POWER:
        supported = ", ".join(sorted(SUPPORTED_POWER))
        raise ValueError(f"Unsupported board.power '{spec.power}'. Supported: {supported}.")

    if spec.mcu_module not in SUPPORTED_MCU:
        supported = ", ".join(sorted(SUPPORTED_MCU))
        raise ValueError(f"Unsupported mcu.module '{spec.mcu_module}'. Supported: {supported}.")

    validate_modules(spec.modules)

    if not any((spec.netlist, spec.schematic, spec.pcb)):
        raise ValueError("At least one output must be enabled in [outputs].")


def render_main_py(spec: BoardSpec) -> str:
    active_modules = tuple(ordered_modules_for(spec.modules))
    resources = ordered_resources_for(active_modules)
    interface_imports = []
    for resource in resources:
        interface_import = RESOURCE_DEFS[resource].interface_import
        if interface_import and interface_import not in interface_imports:
            interface_imports.append(interface_import)

    setup_lines = [RESOURCE_DEFS[resource].setup_line for resource in resources]
    module_imports = []
    for module_name in active_modules:
        import_name = MODULE_DEFS[module_name].import_name
        if import_name not in module_imports:
            module_imports.append(import_name)
    module_lines = [render_call_line(module_name, active_modules) for module_name in active_modules]

    output_lines = ['    print("\\n--- Uruchamiam ERC ---")', "    ERC()"]
    if spec.netlist:
        output_lines.extend(
            [
                '    print("--- Generuję Netlistę ---")',
                '    generate_netlist(file_=str(out_dir / "project.net"))',
            ]
        )
    if spec.schematic:
        output_lines.extend(
            [
                '    print("--- Generuję Schematic ---")',
                "    try:",
                '        generate_schematic(filepath=str(out_dir))',
                "    except Exception as exc:",
                '        print(f"--- Pomijam schematic: {exc}")',
            ]
        )
    if spec.pcb:
        output_lines.extend(
            [
                '    print("--- Generuję PCB ---")',
                "    try:",
                '        generate_pcb(file_=str(out_dir / "project.kicad_pcb"))',
                "    except Exception as exc:",
                '        print(f"--- Pomijam PCB: {exc}")',
            ]
        )

    output_lines.append('    print(f"Sukces! Pliki w: {out_dir}")')

    lines = [
        "import os",
        "import sys",
        "from pathlib import Path",
        "",
        "",
        "def _find_repo_root(start: Path) -> Path:",
        "    for candidate in (start, *start.parents):",
        '        if (candidate / "library" / "interfaces.py").exists():',
        "            return candidate",
        '    raise RuntimeError("Unable to find repo root containing library/interfaces.py")',
        "",
        "project_root = Path(__file__).resolve().parents[1]",
        "repo_root = _find_repo_root(project_root)",
        'if str(project_root) not in sys.path:',
        "    sys.path.insert(0, str(project_root))",
        'if str(repo_root) not in sys.path:',
        "    sys.path.insert(0, str(repo_root))",
        "",
        "from config.config import prepare_kicad_env, setup_skidl",
        "",
        'os.environ.setdefault("XDG_DATA_HOME", str(project_root / ".skidl"))',
        'os.environ.setdefault("MPLCONFIGDIR", str(project_root / ".mplconfig"))',
        "os.chdir(project_root)",
        "",
        "prepare_kicad_env()",
        "",
        "from skidl import *",
        f'from library.interfaces import {", ".join(interface_imports)}',
        f'from library.modules import {", ".join(module_imports)}',
        "",
        "",
        "def main():",
        "    setup_skidl()",
        "",
    ]
    lines.extend(setup_lines)
    lines.append("")
    lines.extend(module_lines)
    lines.extend(
        [
            "",
            '    out_dir = project_root / "out"',
            "    out_dir.mkdir(parents=True, exist_ok=True)",
            "",
        ]
    )
    lines.extend(output_lines)
    lines.extend(
        [
            "",
            "",
            'if __name__ == "__main__":',
            "    main()",
        ]
    )

    return "\n".join(lines) + "\n"


def render_readme(spec: BoardSpec) -> str:
    modules = ", ".join(f"`{module}`" for module in spec.module_names)
    outputs = []
    if spec.netlist:
        outputs.append("netlist")
    if spec.schematic:
        outputs.append("schematic")
    if spec.pcb:
        outputs.append("pcb")

    return dedent(
        f"""\
        # {spec.name}

        {spec.description}

        ## Manifest
        - Power: `{spec.power}`
        - MCU: `{spec.mcu_module}`
        - Modules: {modules}
        - Outputs: `{", ".join(outputs)}`

        ## Workflow
        1. Edytuj `board.toml` i listę modułów w `[composition].modules`.
        2. Uruchom z repo root: `python3 scripts/render_board.py {spec.name}`.
        3. Jeśli dodajesz nowy moduł, dopisz jego kontrakt w `docs/modules/<module_name>/README.md`.
        4. Ustaw `KICAD_SYMBOL_DIR` lub dopasuj `config/config.py`.
        5. Uruchom `python3 src/main.py`.

        ## Uwagi
        - Framework renderuje `src/main.py` z deklaracji w `board.toml`.
        - Kod płytki importuje współdzielone kontrakty z `library/interfaces.py` i moduły z `library/modules/`.
        - W tej iteracji wspierane są tylko płytki ESP32-S3 z zasilaniem `usb_5v`.
        """
    )


def render_board(board_dir: Path):
    board_dir = Path(board_dir).resolve()
    manifest_path = board_dir / "board.toml"
    if not manifest_path.exists():
        raise FileNotFoundError(f"Missing manifest: {manifest_path}")

    spec = load_spec(manifest_path)
    if board_dir.name != spec.name:
        raise ValueError(
            f"board.toml name '{spec.name}' must match directory name '{board_dir.name}'."
        )

    (board_dir / "src" / "main.py").write_text(render_main_py(spec))
    (board_dir / "README.md").write_text(render_readme(spec))
