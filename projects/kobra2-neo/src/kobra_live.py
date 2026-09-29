"""Bounded live executor for an already prepared Kobra 2 Neo pen plot.

This module deliberately does not generate artwork. It revalidates a prepared
G-code/report pair, identifies the physical printer with M115, verifies the
stock Marlin thermal monitor is healthy, performs the explicitly approved
homing preamble, and streams one command at a time while waiting for Marlin
acknowledgements.

It uses only the Python standard library so the live path does not depend on a
second serial package being present in the hardware-lab worker.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import select
import sys
import termios
import time
import tomllib
from typing import Any, Iterable

ALLOWED_ARTWORK_COMMANDS = {"G90", "G0", "G1", "M400"}
FORBIDDEN_ARTWORK_COMMANDS = {"G28", "G91", "M104", "M109", "M140", "M190"}
PARAM_RE = re.compile(r"^([A-Z])([-+]?(?:\d+(?:\.\d*)?|\.\d+))$")
HOTEND_TEMP_RE = re.compile(r"(?:^|\s)T:\s*(-?\d+(?:\.\d+)?)", re.IGNORECASE)
DEFAULT_PROFILE = Path(__file__).resolve().parents[1] / "config" / "kobra2_neo_pen.toml"
PROGRESS_PREFIX = "[AGENT_PROGRESS] "


class LivePlotError(RuntimeError):
    """Fail-closed live plotting error."""


class FirmwareHaltError(LivePlotError):
    """Marlin entered a halted/kill state where further motion is not reliable."""


def _progress(phase: str, message: str, *, current: int | None = None, total: int | None = None, metrics: dict[str, Any] | None = None) -> None:
    payload: dict[str, Any] = {
        "stage_name": "kobra-live",
        "stage_phase": phase,
        "message": message,
    }
    if current is not None:
        payload["current"] = current
    if total is not None:
        payload["total"] = total
    if metrics:
        payload["metrics"] = metrics
    print(PROGRESS_PREFIX + json.dumps(payload, separators=(",", ":"), sort_keys=True), flush=True)


def _load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise LivePlotError(f"cannot read valid JSON report {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise LivePlotError("report root must be an object")
    return value


def _load_profile(path: Path) -> dict[str, Any]:
    try:
        value = tomllib.loads(path.read_text(encoding="utf-8"))
    except (OSError, tomllib.TOMLDecodeError) as exc:
        raise LivePlotError(f"cannot read valid profile {path}: {exc}") from exc
    try:
        workspace = value["workspace"]
        motion = value["motion"]
        tool = value["tool"]
        return {
            "min_x": float(workspace["min_x"]) + float(workspace["margin"]),
            "max_x": float(workspace["max_x"]) - float(workspace["margin"]),
            "min_y": float(workspace["min_y"]) + float(workspace["margin"]),
            "max_y": float(workspace["max_y"]) - float(workspace["margin"]),
            "pen_up_z": float(tool["pen_up_z"]),
            "pen_down_z": float(tool["pen_down_z"]),
            "z_feed": float(motion["z_feed"]),
        }
    except (KeyError, TypeError, ValueError) as exc:
        raise LivePlotError(f"profile is missing required plotting keys: {exc}") from exc


def _active_lines(text: str) -> list[str]:
    active: list[str] = []
    for raw in text.splitlines():
        code = raw.split(";", 1)[0].strip().upper()
        if code:
            active.append(code)
    return active


def _parse_motion(line: str) -> tuple[str, dict[str, float]]:
    parts = line.split()
    command = parts[0]
    params: dict[str, float] = {}
    for token in parts[1:]:
        match = PARAM_RE.fullmatch(token)
        if match is None:
            raise LivePlotError(f"malformed G-code token: {token!r} in {line!r}")
        key, raw_value = match.groups()
        if key not in {"X", "Y", "Z", "F"}:
            raise LivePlotError(f"forbidden G-code parameter {key!r} in {line!r}")
        if key in params:
            raise LivePlotError(f"duplicate G-code parameter {key!r} in {line!r}")
        value = float(raw_value)
        if not math.isfinite(value):
            raise LivePlotError(f"non-finite G-code value in {line!r}")
        params[key] = value
    return command, params


def validate_job(gcode_path: Path, report_path: Path, profile_path: Path, *, expected_sha256: str | None = None) -> dict[str, Any]:
    try:
        raw = gcode_path.read_bytes()
    except OSError as exc:
        raise LivePlotError(f"cannot read G-code {gcode_path}: {exc}") from exc
    digest = hashlib.sha256(raw).hexdigest()
    if expected_sha256 is not None and digest.lower() != expected_sha256.lower():
        raise LivePlotError(f"G-code SHA-256 mismatch: expected {expected_sha256}, got {digest}")

    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise LivePlotError("G-code must be UTF-8 text") from exc

    report = _load_json(report_path)
    profile = _load_profile(profile_path)
    safety = report.get("safety")
    if not isinstance(safety, dict) or safety.get("valid") is not True:
        raise LivePlotError("report does not declare safety.valid=true")
    if safety.get("homing_present") is not False:
        raise LivePlotError("report must declare homing_present=false")
    if safety.get("heater_commands_present") is not False:
        raise LivePlotError("report must declare heater_commands_present=false")
    if safety.get("extrusion_present") is not False:
        raise LivePlotError("report must declare extrusion_present=false")

    allowed_from_report = safety.get("command_whitelist")
    if allowed_from_report is not None and set(allowed_from_report) != ALLOWED_ARTWORK_COMMANDS:
        raise LivePlotError("report command whitelist does not match the live policy")

    lines = _active_lines(text)
    if not lines:
        raise LivePlotError("G-code has no active commands")
    if lines[0] != "G90":
        raise LivePlotError("artwork must start in absolute mode with G90")
    if lines[-1] != "M400":
        raise LivePlotError("artwork must finish with M400")

    x_values: list[float] = []
    y_values: list[float] = []
    for line in lines:
        command = line.split()[0]
        if command in FORBIDDEN_ARTWORK_COMMANDS or command not in ALLOWED_ARTWORK_COMMANDS:
            raise LivePlotError(f"forbidden artwork command: {command}")
        if command in {"G90", "M400"}:
            if line != command:
                raise LivePlotError(f"unexpected parameters on {command}: {line!r}")
            continue
        _, params = _parse_motion(line)
        if "X" in params:
            x = params["X"]
            if not profile["min_x"] <= x <= profile["max_x"]:
                raise LivePlotError(f"X={x} outside normal plotting envelope")
            x_values.append(x)
        if "Y" in params:
            y = params["Y"]
            if not profile["min_y"] <= y <= profile["max_y"]:
                raise LivePlotError(f"Y={y} outside normal plotting envelope")
            y_values.append(y)
        if "Z" in params:
            z = params["Z"]
            if not any(math.isclose(z, allowed, abs_tol=1e-6) for allowed in (profile["pen_up_z"], profile["pen_down_z"])):
                raise LivePlotError(f"unexpected Z={z}; only calibrated pen-up/down values are allowed")

    if not x_values or not y_values:
        raise LivePlotError("artwork contains no bounded XY motion")

    actual_bounds = {
        "min_x": min(x_values),
        "max_x": max(x_values),
        "min_y": min(y_values),
        "max_y": max(y_values),
    }
    report_bounds = report.get("bounds_mm")
    if isinstance(report_bounds, dict):
        for key, actual in actual_bounds.items():
            if key in report_bounds and not math.isclose(float(report_bounds[key]), actual, abs_tol=0.02):
                raise LivePlotError(f"report/G-code bounds mismatch for {key}: report={report_bounds[key]} actual={actual}")

    return {
        "sha256": digest,
        "commands": lines,
        "bounds": actual_bounds,
        "profile": profile,
        "title": report.get("title"),
    }


def extract_hotend_temperature(lines: Iterable[str]) -> float:
    joined = " ".join(lines)
    match = HOTEND_TEMP_RE.search(joined)
    if match is None:
        raise LivePlotError(f"M105 response did not contain hotend temperature: {joined}")
    value = float(match.group(1))
    if not math.isfinite(value):
        raise LivePlotError("M105 returned a non-finite hotend temperature")
    return value


class PosixSerial:
    """Minimal raw 8N1 serial transport for the approved 115200-baud Mac/Linux path."""

    def __init__(self, path: str, baud: int = 115200) -> None:
        if baud != 115200:
            raise LivePlotError("golden Kobra live path currently requires 115200 baud")
        real = os.path.realpath(path)
        if not real.startswith("/dev/"):
            raise LivePlotError("serial port must resolve below /dev")
        self.path = path
        self.fd = os.open(path, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        attrs = termios.tcgetattr(self.fd)
        attrs[0] = 0
        attrs[1] = 0
        attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
        attrs[3] = 0
        attrs[4] = termios.B115200
        attrs[5] = termios.B115200
        attrs[6][termios.VMIN] = 0
        attrs[6][termios.VTIME] = 0
        termios.tcsetattr(self.fd, termios.TCSANOW, attrs)
        termios.tcflush(self.fd, termios.TCIOFLUSH)
        self._buffer = bytearray()

    def close(self) -> None:
        if self.fd >= 0:
            os.close(self.fd)
            self.fd = -1

    def __enter__(self) -> "PosixSerial":
        return self

    def __exit__(self, exc_type: object, exc: object, tb: object) -> None:
        self.close()

    def _write_all(self, payload: bytes, deadline: float) -> None:
        offset = 0
        while offset < len(payload):
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise LivePlotError("serial write timeout")
            _, writable, _ = select.select([], [self.fd], [], remaining)
            if not writable:
                continue
            offset += os.write(self.fd, payload[offset:])

    def _read_line(self, deadline: float) -> str | None:
        while True:
            newline = self._buffer.find(b"\n")
            if newline >= 0:
                raw = bytes(self._buffer[: newline + 1])
                del self._buffer[: newline + 1]
                return raw.decode("utf-8", errors="replace").strip()
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                return None
            readable, _, _ = select.select([self.fd], [], [], remaining)
            if not readable:
                continue
            chunk = os.read(self.fd, 4096)
            if chunk:
                self._buffer.extend(chunk)

    def transact(self, command: str, *, timeout: float) -> list[str]:
        deadline = time.monotonic() + timeout
        self._write_all((command.rstrip() + "\n").encode("ascii"), deadline)
        lines: list[str] = []
        while True:
            line = self._read_line(deadline)
            if line is None:
                raise LivePlotError(f"timeout waiting for Marlin acknowledgement after {command!r}")
            if not line:
                continue
            lines.append(line)
            lower = line.lower()
            if "mintemp" in lower or "maxtemp" in lower or "printer halted" in lower or "kill() called" in lower:
                raise FirmwareHaltError(f"Marlin halted after {command!r}: {line}")
            if lower.startswith("error") or line.startswith("!!"):
                raise LivePlotError(f"Marlin rejected {command!r}: {line}")
            if lower.startswith("resend") or lower.startswith("rs "):
                raise LivePlotError(f"unexpected resend request after {command!r}: {line}")
            if lower == "ok" or lower.startswith("ok "):
                return lines


def _identify(serial: PosixSerial) -> list[str]:
    response = serial.transact("M115", timeout=12.0)
    joined = "\n".join(response).upper()
    if "FIRMWARE_NAME:" not in joined or "ANYCUBICKOBRA" not in joined:
        raise LivePlotError("M115 did not identify the expected Anycubic Kobra printer")
    return response


def _check_thermal_monitor(serial: PosixSerial) -> float:
    response = serial.transact("M105", timeout=10.0)
    hotend_c = extract_hotend_temperature(response)
    # Dedicated pen plotter: hotend/heater/thermistor/fans are intentionally absent.
    # T:0.00 is therefore normal. Firmware-reported MINTEMP/MAXTEMP/kill remains terminal.
    if math.isclose(hotend_c, 0.0, abs_tol=0.01):
        return hotend_c
    if not 5.0 <= hotend_c <= 80.0:
        raise LivePlotError(f"hotend thermal monitor implausible for pen plotting: {hotend_c:.2f} C")
    return hotend_c


def execute(job: dict[str, Any], port: str, *, progress_every: int = 100, thermal_check_every: int = 250) -> float:
    commands: list[str] = job["commands"]
    pen_up_z = job["profile"]["pen_up_z"]
    z_feed = job["profile"]["z_feed"]
    started = time.monotonic()
    homed_z = False
    drawing_started = False
    with PosixSerial(port) as serial:
        # Give an already-open CH340/Marlin session a short deterministic settle.
        time.sleep(0.5)
        _identify(serial)
        _progress("identity", "PRINTER_IDENTIFIED", current=0, total=len(commands), metrics={"port": port})
        hotend_c = _check_thermal_monitor(serial)
        _progress("thermal", "THERMAL_MONITOR_OK", current=0, total=len(commands), metrics={"hotend_c": hotend_c})

        serial.transact("G28 X Y", timeout=120.0)
        _progress("homing", "HOMING_XY_OK", current=0, total=len(commands))
        serial.transact("G28 Z", timeout=120.0)
        homed_z = True
        _progress("homing", "HOMING_Z_OK", current=0, total=len(commands))

        serial.transact("G90", timeout=10.0)
        serial.transact(f"G0 Z{pen_up_z:.2f} F{z_feed:.0f}", timeout=30.0)
        _progress("start", "PEN_UP_OK", current=0, total=len(commands))

        try:
            for index, command in enumerate(commands, start=1):
                timeout = 180.0 if command == "M400" else 60.0
                serial.transact(command, timeout=timeout)
                if index % thermal_check_every == 0 and index < len(commands):
                    hotend_c = _check_thermal_monitor(serial)
                    _progress("thermal", f"THERMAL_MONITOR_OK {index}/{len(commands)}", current=index, total=len(commands), metrics={"hotend_c": hotend_c})
                if not drawing_started and command.startswith("G1 "):
                    drawing_started = True
                    _progress("stream", "DRAWING_STARTED", current=index, total=len(commands))
                elif index % progress_every == 0 or index == len(commands):
                    _progress("stream", f"DRAWING {index}/{len(commands)}", current=index, total=len(commands))

            serial.transact(f"G0 Z{pen_up_z:.2f} F{z_feed:.0f}", timeout=30.0)
            serial.transact("M400", timeout=180.0)
        except FirmwareHaltError:
            # A Marlin kill state rejects motion. Do not claim or repeatedly
            # attempt pen-up when firmware has explicitly halted the system.
            _progress("error", "FIRMWARE_HALTED_FINAL_PEN_STATE_UNKNOWN", current=0, total=len(commands))
            raise
        except BaseException:
            if homed_z:
                try:
                    serial.transact("G90", timeout=5.0)
                    serial.transact(f"G0 Z{pen_up_z:.2f} F{z_feed:.0f}", timeout=15.0)
                    _progress("error", "ABORT_PEN_UP_OK", current=0, total=len(commands))
                except Exception as recovery_exc:  # pragma: no cover - hardware-only recovery
                    _progress("error", "ABORT_PEN_UP_UNCONFIRMED", current=0, total=len(commands))
                    print(f"RECOVERY_ERROR:{recovery_exc}", file=sys.stderr, flush=True)
            raise

    elapsed = time.monotonic() - started
    _progress("complete", "COMPLETE_PEN_UP", current=len(commands), total=len(commands), metrics={"elapsed_seconds": round(elapsed, 3)})
    return elapsed


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Validate and stream one approved Kobra 2 Neo pen plot")
    parser.add_argument("gcode", type=Path, help="prepared G-code file")
    parser.add_argument("--report", type=Path, required=True, help="matching safety report JSON")
    parser.add_argument("--profile", type=Path, default=DEFAULT_PROFILE, help="calibrated Kobra pen profile")
    parser.add_argument("--port", help="explicit serial device, for example /dev/cu.usbserial-130")
    parser.add_argument("--expect-sha256", help="optional immutable G-code SHA-256 pin")
    parser.add_argument("--validate-only", action="store_true", help="perform full offline preflight and never open serial")
    parser.add_argument("--progress-every", type=int, default=100, help="emit structured stream progress every N commands")
    parser.add_argument("--thermal-check-every", type=int, default=250, help="query the Marlin thermal monitor every N streamed commands")
    return parser


def main(argv: Iterable[str] | None = None) -> int:
    args = _parser().parse_args(list(argv) if argv is not None else None)
    if args.progress_every <= 0:
        raise SystemExit("--progress-every must be > 0")
    if args.thermal_check_every <= 0:
        raise SystemExit("--thermal-check-every must be > 0")
    try:
        job = validate_job(args.gcode, args.report, args.profile, expected_sha256=args.expect_sha256)
        _progress(
            "preflight",
            "PREFLIGHT_OK",
            current=0,
            total=len(job["commands"]),
            metrics={"sha256": job["sha256"], **job["bounds"]},
        )
        if args.validate_only:
            print(f"RESULT:KOBRA_LIVE_PREFLIGHT_OK sha256={job['sha256']} commands={len(job['commands'])}")
            return 0
        if not args.port:
            raise LivePlotError("--port is required unless --validate-only is used")
        elapsed = execute(
            job,
            args.port,
            progress_every=args.progress_every,
            thermal_check_every=args.thermal_check_every,
        )
        print(f"RESULT:KOBRA_LIVE_COMPLETE_PEN_UP elapsed_seconds={elapsed:.3f}")
        return 0
    except LivePlotError as exc:
        _progress("error", f"BLOCKED: {exc}")
        print(f"ERROR:{exc}", file=sys.stderr, flush=True)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
