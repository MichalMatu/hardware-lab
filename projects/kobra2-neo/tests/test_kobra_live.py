from __future__ import annotations

import hashlib
import json
from pathlib import Path

import pytest

from kobra_live import LivePlotError, PROGRESS_PREFIX, main, validate_job


PROFILE = Path(__file__).resolve().parents[1] / "config" / "kobra2_neo_pen.toml"


def _job(tmp_path: Path, gcode: str) -> tuple[Path, Path]:
    gcode_path = tmp_path / "job.gcode"
    report_path = tmp_path / "job.report.json"
    gcode_path.write_text(gcode, encoding="utf-8")
    report = {
        "safety": {
            "valid": True,
            "homing_present": False,
            "heater_commands_present": False,
            "extrusion_present": False,
            "command_whitelist": ["G90", "G0", "G1", "M400"],
        },
        "bounds_mm": {"min_x": 10.0, "max_x": 20.0, "min_y": 50.0, "max_y": 60.0},
    }
    report_path.write_text(json.dumps(report), encoding="utf-8")
    return gcode_path, report_path


def test_validate_known_safe_job(tmp_path: Path) -> None:
    gcode, report = _job(
        tmp_path,
        "G90\nG0 Z6.12 F180\nG0 X10 Y50 F3000\nG0 Z2.97 F180\nG1 X20 Y60 F1200\nG0 Z6.12 F180\nM400\n",
    )
    result = validate_job(gcode, report, PROFILE)
    assert len(result["commands"]) == 7
    assert result["bounds"] == {"min_x": 10.0, "max_x": 20.0, "min_y": 50.0, "max_y": 60.0}
    assert result["sha256"] == hashlib.sha256(gcode.read_bytes()).hexdigest()


def test_validate_rejects_homing_in_artwork(tmp_path: Path) -> None:
    gcode, report = _job(
        tmp_path,
        "G90\nG28 X Y\nG0 X10 Y50\nG1 X20 Y60\nM400\n",
    )
    with pytest.raises(LivePlotError, match="forbidden artwork command"):
        validate_job(gcode, report, PROFILE)


def test_validate_rejects_motion_outside_normal_envelope(tmp_path: Path) -> None:
    gcode, report = _job(
        tmp_path,
        "G90\nG0 X7 Y50\nG1 X20 Y60\nM400\n",
    )
    with pytest.raises(LivePlotError, match="outside normal plotting envelope"):
        validate_job(gcode, report, PROFILE)


def test_validate_rejects_unexpected_z(tmp_path: Path) -> None:
    gcode, report = _job(
        tmp_path,
        "G90\nG0 X10 Y50\nG0 Z4.00\nG1 X20 Y60\nM400\n",
    )
    with pytest.raises(LivePlotError, match="unexpected Z"):
        validate_job(gcode, report, PROFILE)


def test_validate_sha_pin(tmp_path: Path) -> None:
    gcode, report = _job(
        tmp_path,
        "G90\nG0 X10 Y50\nG1 X20 Y60\nM400\n",
    )
    with pytest.raises(LivePlotError, match="SHA-256 mismatch"):
        validate_job(gcode, report, PROFILE, expected_sha256="0" * 64)


def test_cli_validate_only_emits_agent_progress(tmp_path: Path, capsys: pytest.CaptureFixture[str]) -> None:
    gcode, report = _job(
        tmp_path,
        "G90\nG0 Z6.12 F180\nG0 X10 Y50 F3000\nG0 Z2.97 F180\nG1 X20 Y60 F1200\nG0 Z6.12 F180\nM400\n",
    )
    assert main([str(gcode), "--report", str(report), "--validate-only"]) == 0
    stdout = capsys.readouterr().out
    assert PROGRESS_PREFIX in stdout
    assert '"message":"PREFLIGHT_OK"' in stdout
    assert "RESULT:KOBRA_LIVE_PREFLIGHT_OK" in stdout
