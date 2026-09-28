#!/usr/bin/env python3

import argparse
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from framework.generator import render_board


def parse_args():
    parser = argparse.ArgumentParser(
        description="Render board.toml into src/main.py and README.md.",
    )
    parser.add_argument(
        "board_dir",
        help="Board name from boards/, board directory, or direct path to board.toml.",
    )
    return parser.parse_args()


def resolve_board_path(raw_board_path: str) -> Path:
    candidate = Path(raw_board_path).expanduser()
    if candidate.exists():
        return candidate.resolve()

    board_in_repo = ROOT / "boards" / raw_board_path
    if board_in_repo.exists():
        return board_in_repo.resolve()

    raise FileNotFoundError(f"Board path not found: {raw_board_path}")


def main():
    args = parse_args()
    board_path = resolve_board_path(args.board_dir)
    if board_path.name == "board.toml":
        board_path = board_path.parent

    render_board(board_path)
    print(f"Rendered board from: {board_path / 'board.toml'}")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        sys.exit(1)
