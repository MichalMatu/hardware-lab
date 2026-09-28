from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import hashlib
from html import escape
import json
import math
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import tomllib
from typing import Literal
import xml.etree.ElementTree as ET

__version__ = "0.1.0"

Point = tuple[float, float]
Polyline = list[Point]
SourceKind = Literal["svg", "text"]

_TOKEN_RE = re.compile(r"[MmLlHhVvZz]|[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?")
_WORD_RE = re.compile(r"^([A-Za-z])([-+]?(?:\d+(?:\.\d*)?|\.\d+))$")


class KobraPlotError(RuntimeError):
    pass


class ProfileError(KobraPlotError):
    pass


class GeometryError(KobraPlotError):
    pass


class GCodeSafetyError(KobraPlotError):
    pass


@dataclass(frozen=True)
class Workspace:
    min_x: float
    max_x: float
    min_y: float
    max_y: float
    margin: float

    @property
    def draw_min_x(self) -> float:
        return self.min_x + self.margin

    @property
    def draw_max_x(self) -> float:
        return self.max_x - self.margin

    @property
    def draw_min_y(self) -> float:
        return self.min_y + self.margin

    @property
    def draw_max_y(self) -> float:
        return self.max_y - self.margin


@dataclass(frozen=True)
class Motion:
    travel_feed: float
    draw_feed: float
    z_feed: float


@dataclass(frozen=True)
class Orientation:
    swap_xy: bool
    flip_x: bool
    flip_y: bool


@dataclass(frozen=True)
class Tool:
    pen_up_z: float
    pen_down_z: float


@dataclass(frozen=True)
class Profile:
    schema_version: int
    name: str
    workspace: Workspace
    motion: Motion
    orientation: Orientation
    tool: Tool
    end_commands: tuple[str, ...]


@dataclass(frozen=True)
class Bounds:
    min_x: float
    min_y: float
    max_x: float
    max_y: float

    @property
    def width(self) -> float:
        return self.max_x - self.min_x

    @property
    def height(self) -> float:
        return self.max_y - self.min_y

    def as_dict(self) -> dict[str, float]:
        return {
            "min_x": self.min_x,
            "min_y": self.min_y,
            "max_x": self.max_x,
            "max_y": self.max_y,
            "width": self.width,
            "height": self.height,
        }


_ROOT_KEYS = {"schema_version", "name", "workspace", "motion", "orientation", "tool", "sequence"}
_SECTION_KEYS = {
    "workspace": {"min_x", "max_x", "min_y", "max_y", "margin"},
    "motion": {"travel_feed", "draw_feed", "z_feed"},
    "orientation": {"swap_xy", "flip_x", "flip_y"},
    "tool": {"pen_up_z", "pen_down_z"},
    "sequence": {"end"},
}


def project_root() -> Path:
    return Path(__file__).resolve().parents[1]


def default_profile_path() -> Path:
    return project_root() / "config" / "kobra2_neo_pen.toml"


def _require_exact_keys(name: str, value: dict, expected: set[str]) -> None:
    unknown = set(value) - expected
    missing = expected - set(value)
    if unknown:
        raise ProfileError(f"{name}: unknown keys: {', '.join(sorted(unknown))}")
    if missing:
        raise ProfileError(f"{name}: missing keys: {', '.join(sorted(missing))}")


def load_profile(path: Path | None = None) -> Profile:
    profile_path = (path or default_profile_path()).expanduser().resolve()
    try:
        with profile_path.open("rb") as handle:
            raw = tomllib.load(handle)
    except OSError as exc:
        raise ProfileError(f"cannot read profile {profile_path}: {exc}") from exc

    _require_exact_keys("profile", raw, _ROOT_KEYS)
    if raw["schema_version"] != 1:
        raise ProfileError(f"unsupported schema_version: {raw['schema_version']!r}")

    for section, keys in _SECTION_KEYS.items():
        if not isinstance(raw[section], dict):
            raise ProfileError(f"{section}: expected table")
        _require_exact_keys(section, raw[section], keys)

    ws = Workspace(**{k: float(raw["workspace"][k]) for k in _SECTION_KEYS["workspace"]})
    motion = Motion(**{k: float(raw["motion"][k]) for k in _SECTION_KEYS["motion"]})
    orientation = Orientation(**{k: bool(raw["orientation"][k]) for k in _SECTION_KEYS["orientation"]})
    tool = Tool(**{k: float(raw["tool"][k]) for k in _SECTION_KEYS["tool"]})
    end = raw["sequence"]["end"]

    if not isinstance(end, list) or not all(isinstance(item, str) and item.strip() for item in end):
        raise ProfileError("sequence.end must be a list of non-empty strings")
    if not (ws.min_x < ws.max_x and ws.min_y < ws.max_y):
        raise ProfileError("workspace bounds are invalid")
    if ws.margin < 0 or ws.draw_min_x >= ws.draw_max_x or ws.draw_min_y >= ws.draw_max_y:
        raise ProfileError("workspace margin leaves no drawable area")
    if min(motion.travel_feed, motion.draw_feed, motion.z_feed) <= 0:
        raise ProfileError("feed rates must be positive")
    if tool.pen_down_z < 0 or tool.pen_up_z <= tool.pen_down_z:
        raise ProfileError("pen Z values are invalid")

    return Profile(
        schema_version=1,
        name=str(raw["name"]),
        workspace=ws,
        motion=motion,
        orientation=orientation,
        tool=tool,
        end_commands=tuple(item.strip() for item in end),
    )


def vpype_executable() -> Path:
    override = os.environ.get("KOBRA_PLOT_VPYPE")
    if override:
        path = Path(override).expanduser()
        if path.is_file():
            return path
        raise KobraPlotError(f"KOBRA_PLOT_VPYPE does not point to a file: {path}")

    sibling = Path(sys.executable).resolve().parent / "vpype"
    if sibling.is_file():
        return sibling

    found = shutil.which("vpype")
    if found:
        return Path(found)

    raise KobraPlotError(
        "vpype is unavailable; from projects/kobra2-neo run `uv sync` "
        "and then use `uv run kobra-plot ...`"
    )


def vpype_version() -> str:
    proc = subprocess.run(
        [str(vpype_executable()), "--version"],
        capture_output=True,
        text=True,
        check=False,
    )
    if proc.returncode != 0:
        raise KobraPlotError((proc.stderr or proc.stdout).strip() or "vpype --version failed")
    return (proc.stdout or proc.stderr).strip()


def _run_vpype(args: list[str]) -> None:
    proc = subprocess.run(
        [str(vpype_executable()), *args],
        capture_output=True,
        text=True,
        check=False,
    )
    if proc.returncode != 0:
        detail = (proc.stderr or proc.stdout).strip()
        raise KobraPlotError(f"vpype failed ({proc.returncode}): {detail}")


def normalize_svg(source: Path, output: Path) -> None:
    _run_vpype([
        "read",
        "--single-layer",
        "--simplify",
        "--quantization",
        "0.10mm",
        str(source),
        "linesimplify",
        "linesort",
        "write",
        "--color-mode",
        "none",
        "--dont-set-date",
        str(output),
    ])


def normalize_text(text: str, output: Path, font: str = "scripts") -> None:
    text = " ".join(text.split())
    if not text:
        raise KobraPlotError("text input is empty")
    _run_vpype([
        "text",
        "--font",
        font,
        "--size",
        "18",
        text,
        "linesimplify",
        "linesort",
        "write",
        "--color-mode",
        "none",
        "--dont-set-date",
        str(output),
    ])


def _parse_points(value: str) -> list[Point]:
    nums = [
        float(tok)
        for tok in re.findall(r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?", value)
    ]
    if len(nums) % 2:
        raise GeometryError("odd coordinate count in points attribute")
    return list(zip(nums[0::2], nums[1::2], strict=True))


def parse_linear_path(d: str) -> list[Polyline]:
    tokens = _TOKEN_RE.findall(d.replace(",", " "))
    if not tokens:
        return []

    out: list[Polyline] = []
    poly: Polyline = []
    current: Point = (0.0, 0.0)
    start: Point | None = None
    cmd: str | None = None
    i = 0

    def finish() -> None:
        nonlocal poly
        if len(poly) >= 2:
            out.append(poly)
        poly = []

    def number(index: int) -> float:
        if index >= len(tokens) or tokens[index].isalpha():
            raise GeometryError(f"expected numeric path argument near token {index}")
        return float(tokens[index])

    while i < len(tokens):
        token = tokens[i]
        if token.isalpha():
            cmd = token
            i += 1
            if cmd in "Zz":
                if poly and start is not None and poly[-1] != start:
                    poly.append(start)
                finish()
                current = start or current
                start = None
                cmd = None
            continue

        if cmd is None:
            raise GeometryError(f"missing path command near token {i}")

        relative = cmd.islower()
        upper = cmd.upper()
        if upper in {"M", "L"}:
            x = number(i)
            y = number(i + 1)
            i += 2
            if relative:
                x += current[0]
                y += current[1]
            current = (x, y)
            if upper == "M":
                finish()
                poly = [current]
                start = current
                cmd = "l" if relative else "L"
            else:
                if not poly:
                    poly = [current]
                    start = current
                else:
                    poly.append(current)
        elif upper == "H":
            x = number(i)
            i += 1
            if relative:
                x += current[0]
            current = (x, current[1])
            if not poly:
                poly = [current]
                start = current
            else:
                poly.append(current)
        elif upper == "V":
            y = number(i)
            i += 1
            if relative:
                y += current[1]
            current = (current[0], y)
            if not poly:
                poly = [current]
                start = current
            else:
                poly.append(current)
        else:
            raise GeometryError(f"non-linear SVG command {cmd!r} found after normalization")

    finish()
    return out


def load_normalized_svg(path: Path) -> list[Polyline]:
    try:
        root = ET.parse(path).getroot()
    except ET.ParseError as exc:
        raise GeometryError(f"invalid normalized SVG: {exc}") from exc

    lines: list[Polyline] = []
    for elem in root.iter():
        tag = elem.tag.rsplit("}", 1)[-1]
        if tag == "path":
            lines.extend(parse_linear_path(elem.get("d", "")))
        elif tag == "line":
            lines.append([
                (float(elem.attrib["x1"]), float(elem.attrib["y1"])),
                (float(elem.attrib["x2"]), float(elem.attrib["y2"])),
            ])
        elif tag in {"polyline", "polygon"}:
            pts = _parse_points(elem.get("points", ""))
            if tag == "polygon" and pts and pts[-1] != pts[0]:
                pts.append(pts[0])
            if len(pts) >= 2:
                lines.append(pts)

    if not lines:
        raise GeometryError("normalized SVG contains no drawable line geometry")
    return lines


def bounds(polylines: list[Polyline]) -> Bounds:
    points = [point for line in polylines for point in line]
    if not points:
        raise GeometryError("geometry is empty")
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    result = Bounds(min(xs), min(ys), max(xs), max(ys))
    if not all(math.isfinite(v) for v in (result.min_x, result.min_y, result.max_x, result.max_y)):
        raise GeometryError("geometry contains non-finite coordinates")
    return result


def fit_to_profile(polylines: list[Polyline], profile: Profile) -> list[Polyline]:
    transformed = [
        [(y, x) if profile.orientation.swap_xy else (x, y) for x, y in line]
        for line in polylines
    ]
    b = bounds(transformed)

    oriented: list[Polyline] = []
    for line in transformed:
        out: Polyline = []
        for x, y in line:
            if profile.orientation.flip_x:
                x = b.max_x - (x - b.min_x)
            if profile.orientation.flip_y:
                y = b.max_y - (y - b.min_y)
            out.append((x, y))
        oriented.append(out)

    b = bounds(oriented)
    if b.width <= 0 and b.height <= 0:
        raise GeometryError("geometry has zero extent")

    ws = profile.workspace
    target_w = ws.draw_max_x - ws.draw_min_x
    target_h = ws.draw_max_y - ws.draw_min_y
    sx = math.inf if b.width == 0 else target_w / b.width
    sy = math.inf if b.height == 0 else target_h / b.height
    scale = min(sx, sy)
    if not math.isfinite(scale) or scale <= 0:
        scale = sx if math.isfinite(sx) else sy
    if not math.isfinite(scale) or scale <= 0:
        raise GeometryError("cannot fit degenerate geometry")

    used_w = b.width * scale
    used_h = b.height * scale
    off_x = ws.draw_min_x + (target_w - used_w) / 2
    off_y = ws.draw_min_y + (target_h - used_h) / 2

    return [
        [
            (off_x + (x - b.min_x) * scale, off_y + (y - b.min_y) * scale)
            for x, y in line
        ]
        for line in oriented
    ]


def _distance(a: Point, b: Point) -> float:
    return math.hypot(b[0] - a[0], b[1] - a[1])


def geometry_stats(polylines: list[Polyline], profile: Profile) -> dict[str, object]:
    draw = 0.0
    travel = 0.0
    z_distance = 0.0
    previous_end: Point | None = None
    for line in polylines:
        if previous_end is not None:
            travel += _distance(previous_end, line[0])
        draw += sum(_distance(a, b) for a, b in zip(line, line[1:]))
        z_distance += abs(profile.tool.pen_up_z - profile.tool.pen_down_z) * 2
        previous_end = line[-1]

    seconds = (
        draw / profile.motion.draw_feed * 60
        + travel / profile.motion.travel_feed * 60
        + z_distance / profile.motion.z_feed * 60
    )
    return {
        "polylines": len(polylines),
        "points": sum(len(line) for line in polylines),
        "draw_distance_mm": round(draw, 3),
        "travel_distance_mm": round(travel, 3),
        "nominal_time_seconds": round(seconds, 2),
        "initial_xy_travel_included_in_estimate": False,
    }


def generate_gcode(polylines: list[Polyline], profile: Profile) -> str:
    out = [
        "; kobra-plot generated offline job",
        f"; profile={profile.name}",
        "; homing intentionally omitted",
        "G90",
        f"G0 Z{profile.tool.pen_up_z:.2f} F{profile.motion.z_feed:.0f}",
    ]
    for line in polylines:
        if len(line) < 2:
            continue
        sx, sy = line[0]
        out.append(f"G0 X{sx:.2f} Y{sy:.2f} F{profile.motion.travel_feed:.0f}")
        out.append(f"G0 Z{profile.tool.pen_down_z:.2f} F{profile.motion.z_feed:.0f}")
        for x, y in line[1:]:
            out.append(f"G1 X{x:.2f} Y{y:.2f} F{profile.motion.draw_feed:.0f}")
        out.append(f"G0 Z{profile.tool.pen_up_z:.2f} F{profile.motion.z_feed:.0f}")
    out.extend(profile.end_commands)
    return "\n".join(out) + "\n"


def _parse_gcode_line(line: str) -> tuple[str, dict[str, float]]:
    content = line.split(";", 1)[0].strip()
    if not content:
        return "", {}
    parts = content.split()
    command = parts[0].upper()
    params: dict[str, float] = {}
    for raw in parts[1:]:
        match = _WORD_RE.match(raw)
        if not match:
            raise GCodeSafetyError(f"malformed G-code token: {raw!r}")
        key = match.group(1).upper()
        if key in params:
            raise GCodeSafetyError(f"duplicate G-code parameter: {key}")
        params[key] = float(match.group(2))
    return command, params


def validate_gcode(text: str, profile: Profile) -> dict[str, object]:
    allowed = {"G90", "G0", "G1", "M400"}
    seen_g90 = False
    current_x: float | None = None
    current_y: float | None = None
    xy_moves = 0

    for line_no, raw in enumerate(text.splitlines(), 1):
        command, params = _parse_gcode_line(raw)
        if not command:
            continue
        if command not in allowed:
            raise GCodeSafetyError(f"line {line_no}: command {command} is not allowed")
        if command in {"G90", "M400"} and params:
            raise GCodeSafetyError(f"line {line_no}: {command} must not have parameters")
        if "E" in params:
            raise GCodeSafetyError(f"line {line_no}: extrusion parameter E is forbidden")
        unknown = set(params) - {"X", "Y", "Z", "F"}
        if unknown:
            raise GCodeSafetyError(f"line {line_no}: unsupported parameters {sorted(unknown)}")

        if command == "G90":
            seen_g90 = True
            continue

        if command in {"G0", "G1"}:
            if not seen_g90:
                raise GCodeSafetyError(f"line {line_no}: motion before G90")
            if "X" in params:
                current_x = params["X"]
            if "Y" in params:
                current_y = params["Y"]
            if "X" in params or "Y" in params:
                xy_moves += 1
                if current_x is None or current_y is None:
                    raise GCodeSafetyError(f"line {line_no}: first XY move must specify X and Y")
                ws = profile.workspace
                if not ws.min_x <= current_x <= ws.max_x:
                    raise GCodeSafetyError(f"line {line_no}: X={current_x} outside hard envelope")
                if not ws.min_y <= current_y <= ws.max_y:
                    raise GCodeSafetyError(f"line {line_no}: Y={current_y} outside hard envelope")
            if "Z" in params:
                z = params["Z"]
                allowed_z = (profile.tool.pen_up_z, profile.tool.pen_down_z)
                if not any(math.isclose(z, value, abs_tol=0.005) for value in allowed_z):
                    raise GCodeSafetyError(f"line {line_no}: unexpected Z={z}")

    if not seen_g90:
        raise GCodeSafetyError("prepared G-code must include G90")
    if xy_moves == 0:
        raise GCodeSafetyError("prepared G-code contains no XY motion")

    return {
        "valid": True,
        "homing_present": False,
        "heater_commands_present": False,
        "extrusion_present": False,
        "xy_moves": xy_moves,
    }


def render_preview(polylines: list[Polyline], profile: Profile) -> str:
    ws = profile.workspace
    width = ws.max_x - ws.min_x
    height = ws.max_y - ws.min_y

    def path(points: list[Point]) -> str:
        mapped = [(x - ws.min_x, ws.max_y - y) for x, y in points]
        return " ".join(
            ("M" if idx == 0 else "L") + f"{x:.2f},{y:.2f}"
            for idx, (x, y) in enumerate(mapped)
        )

    travel_paths = "\n".join(
        f'    <path d="{escape(path([first[-1], second[0]]))}" />'
        for first, second in zip(polylines, polylines[1:])
    )
    draw_paths = "\n".join(
        f'    <path d="{escape(path(line))}" />'
        for line in polylines
    )
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}mm" height="{height}mm" '
        f'viewBox="0 0 {width} {height}">\n'
        f'  <rect x="0" y="0" width="{width}" height="{height}" fill="white" stroke="black" stroke-width="0.3"/>\n'
        '  <g id="travel" fill="none" stroke="#aaaaaa" stroke-width="0.2" stroke-dasharray="1,1">\n'
        f"{travel_paths}\n"
        "  </g>\n"
        '  <g id="draw" fill="none" stroke="black" stroke-width="0.35" stroke-linecap="round" stroke-linejoin="round">\n'
        f"{draw_paths}\n"
        "  </g>\n"
        "</svg>\n"
    )


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def classify_source(path: Path) -> SourceKind:
    suffix = path.suffix.lower()
    if suffix == ".svg":
        return "svg"
    if suffix in {".txt", ".text"}:
        return "text"
    if suffix in {".png", ".jpg", ".jpeg", ".webp", ".bmp", ".tif", ".tiff"}:
        raise KobraPlotError(
            "raster source recognized, but raster rendering is intentionally disabled in V1; "
            "the next phase will add explicit outline/hatch/stipple presets"
        )
    if suffix == ".pdf":
        raise KobraPlotError("PDF source recognized, but PDF conversion is intentionally disabled in V1")
    raise KobraPlotError(f"unsupported source type: {suffix or '<none>'}")


def _profile_report(profile: Profile) -> dict[str, object]:
    ws = profile.workspace
    return {
        "name": profile.name,
        "schema_version": profile.schema_version,
        "hard_envelope": {
            "min_x": ws.min_x,
            "max_x": ws.max_x,
            "min_y": ws.min_y,
            "max_y": ws.max_y,
        },
        "drawing_envelope": {
            "min_x": ws.draw_min_x,
            "max_x": ws.draw_max_x,
            "min_y": ws.draw_min_y,
            "max_y": ws.draw_max_y,
        },
        "pen_up_z": profile.tool.pen_up_z,
        "pen_down_z": profile.tool.pen_down_z,
        "motion": asdict(profile.motion),
        "orientation": asdict(profile.orientation),
    }


def prepare_file(
    source: Path,
    output: Path,
    *,
    profile_path: Path | None = None,
    force: bool = False,
) -> dict[str, object]:
    source = source.expanduser().resolve()
    output = output.expanduser().resolve()
    if not source.is_file():
        raise KobraPlotError(f"source file does not exist: {source}")

    kind = classify_source(source)
    profile = load_profile(profile_path)

    if output.exists():
        if not force:
            raise KobraPlotError(f"output directory already exists: {output}")
        if not output.is_dir():
            raise KobraPlotError(f"output path is not a directory: {output}")
        shutil.rmtree(output)
    output.mkdir(parents=True)

    copied_source = output / f"source{source.suffix.lower()}"
    shutil.copy2(source, copied_source)
    normalized = output / "normalized.svg"

    if kind == "svg":
        normalize_svg(copied_source, normalized)
    else:
        try:
            text = copied_source.read_text(encoding="utf-8")
        except UnicodeError as exc:
            raise KobraPlotError(f"text source is not UTF-8: {exc}") from exc
        normalize_text(text, normalized)

    normalized_geometry = load_normalized_svg(normalized)
    input_bounds = bounds(normalized_geometry)
    fitted = fit_to_profile(normalized_geometry, profile)
    output_bounds = bounds(fitted)

    gcode = generate_gcode(fitted, profile)
    safety = validate_gcode(gcode, profile)

    gcode_path = output / "output.gcode"
    preview_path = output / "preview.svg"
    report_path = output / "report.json"
    gcode_path.write_text(gcode, encoding="utf-8")
    preview_path.write_text(render_preview(fitted, profile), encoding="utf-8")

    stats = geometry_stats(fitted, profile)
    stats["input_bounds"] = input_bounds.as_dict()
    stats["output_bounds"] = output_bounds.as_dict()

    report: dict[str, object] = {
        "schema_version": 1,
        "source": {
            "kind": kind,
            "filename": source.name,
            "sha256": sha256(copied_source),
        },
        "backend": {"vpype": vpype_version()},
        "geometry": stats,
        "profile": _profile_report(profile),
        "safety": safety,
        "artifacts": {
            "normalized_svg": "normalized.svg",
            "preview_svg": "preview.svg",
            "gcode": "output.gcode",
            "gcode_sha256": sha256(gcode_path),
        },
        "execution": {
            "allowed": False,
            "reason": "prepare-only V1 has no live transport or execute command",
        },
    }
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report


def _print_summary(report: dict[str, object], output: Path) -> None:
    geometry = report["geometry"]
    safety = report["safety"]
    assert isinstance(geometry, dict)
    assert isinstance(safety, dict)
    print(f"job: {output}")
    print(
        f"geometry: {geometry['polylines']} polylines, {geometry['points']} points, "
        f"draw {geometry['draw_distance_mm']} mm, travel {geometry['travel_distance_mm']} mm"
    )
    print(f"nominal time: {geometry['nominal_time_seconds']} s")
    print(f"safety: {'PASS' if safety.get('valid') else 'FAIL'}")
    print("execution: disabled (prepare-only V1)")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="kobra-plot",
        description="Offline job preparation for the Kobra 2 Neo pen plotter.",
    )
    parser.add_argument("--version", action="version", version=f"%(prog)s {__version__}")
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("doctor", help="Check offline preparation dependencies.")

    prepare = sub.add_parser("prepare", help="Prepare an SVG or text file offline.")
    prepare.add_argument("source", type=Path)
    prepare.add_argument("-o", "--output", type=Path)
    prepare.add_argument("--profile", type=Path, default=default_profile_path())
    prepare.add_argument("--force", action="store_true")

    text = sub.add_parser("text", help="Prepare literal text using a bundled Hershey font.")
    text.add_argument("text")
    text.add_argument("-o", "--output", type=Path, required=True)
    text.add_argument("--profile", type=Path, default=default_profile_path())
    text.add_argument("--force", action="store_true")

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        if args.command == "doctor":
            print(json.dumps({
                "kobra_plot": __version__,
                "profile": str(default_profile_path()),
                "vpype": vpype_version(),
                "serial_transport": False,
                "execute_command": False,
            }, indent=2, sort_keys=True))
            return 0

        if args.command == "prepare":
            output = args.output or Path.cwd() / f"{args.source.stem}.kobra-job"
            report = prepare_file(
                args.source,
                output,
                profile_path=args.profile,
                force=args.force,
            )
            _print_summary(report, output)
            return 0

        if args.command == "text":
            with tempfile.TemporaryDirectory(prefix="kobra-plot-text-") as tmp:
                source = Path(tmp) / "input.txt"
                source.write_text(args.text + "\n", encoding="utf-8")
                report = prepare_file(
                    source,
                    args.output,
                    profile_path=args.profile,
                    force=args.force,
                )
            _print_summary(report, args.output)
            return 0

        parser.error(f"unsupported command: {args.command}")
    except KobraPlotError as exc:
        parser.error(str(exc))
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
