#!/usr/bin/env python3

import argparse
import re
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from framework.generator import render_board


ROOT = Path(__file__).resolve().parents[1]
TEMPLATE_DIR = ROOT / "boards" / "_template"
DEFAULT_DESTINATION_ROOT = ROOT / "boards"
IGNORED_NAMES = {"__pycache__", ".skidl", ".mplconfig"}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Create a new PCB project from boards/_template.",
    )
    parser.add_argument("board_name", help="Directory name for the new board.")
    parser.add_argument(
        "--destination-root",
        default=str(DEFAULT_DESTINATION_ROOT),
        help="Directory where the new board folder will be created. Supported default: repo/boards.",
    )
    return parser.parse_args()


def validate_board_name(board_name: str):
    if not re.fullmatch(r"[a-z0-9][a-z0-9_-]*", board_name):
        raise ValueError(
            "Board name must use lowercase letters, digits, '_' or '-'."
        )
    if board_name == "_template":
        raise ValueError("Board name '_template' is reserved.")


def ignore_entries(directory, names):
    ignored = {
        name
        for name in names
        if name in IGNORED_NAMES
        or name.endswith(".erc")
        or name.endswith(".log")
        or name.endswith("_sklib.py")
    }
    if Path(directory).name == "out":
        ignored.add("project.net")
    return ignored


def rewrite_placeholders(board_dir: Path, board_name: str):
    manifest_path = board_dir / "board.toml"
    spec_path = board_dir / "spec.md"

    manifest_text = manifest_path.read_text()
    manifest_text = manifest_text.replace("<board_name>", board_name)
    manifest_path.write_text(manifest_text)

    spec_text = spec_path.read_text()
    spec_text = spec_text.replace("<nazwa_plytki>", board_name)
    spec_path.write_text(spec_text)


def main():
    args = parse_args()
    board_name = args.board_name
    validate_board_name(board_name)
    destination_root = Path(args.destination_root).expanduser().resolve()

    target_dir = destination_root / board_name
    if target_dir.exists():
        raise FileExistsError(f"Target already exists: {target_dir}")

    shutil.copytree(TEMPLATE_DIR, target_dir, ignore=ignore_entries)
    rewrite_placeholders(target_dir, board_name)
    render_board(target_dir)

    print(f"Created new board at: {target_dir}")
    print(f"Next: edit {target_dir / 'board.toml'} and rerun scripts/render_board.py {board_name} if needed")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        sys.exit(1)
