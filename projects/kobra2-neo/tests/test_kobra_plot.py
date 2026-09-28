from pathlib import Path

import pytest

import kobra_plot as kp


PROJECT = Path(__file__).resolve().parents[1]
SAMPLE = PROJECT / "samples" / "svg" / "curve-demo.svg"


def test_parse_linear_path_absolute_and_relative() -> None:
    lines = kp.parse_linear_path("M 0 0 L 10 0 l 0 10 h -5 v 5 z")
    assert len(lines) == 1
    assert lines[0][0] == (0.0, 0.0)
    assert lines[0][-1] == (0.0, 0.0)


def test_fit_inside_drawing_envelope() -> None:
    profile = kp.load_profile()
    fitted = kp.fit_to_profile([[(0.0, 0.0), (100.0, 50.0)]], profile)
    result = kp.bounds(fitted)
    ws = profile.workspace
    assert ws.draw_min_x <= result.min_x <= result.max_x <= ws.draw_max_x
    assert ws.draw_min_y <= result.min_y <= result.max_y <= ws.draw_max_y


def test_generated_gcode_is_safe() -> None:
    profile = kp.load_profile()
    gcode = kp.generate_gcode([[(10.0, 50.0), (20.0, 60.0)]], profile)
    safety = kp.validate_gcode(gcode, profile)
    assert safety["valid"] is True
    for forbidden in ("G28", "G91", "M104", "M109", "M140", "M190", " E"):
        assert forbidden not in gcode


@pytest.mark.parametrize(
    "line",
    [
        "G28",
        "G91",
        "M104 S200",
        "M109 S200",
        "M140 S60",
        "M190 S60",
        "G1 X10 Y50 E1 F1200",
        "G1 X999 Y50 F1200",
    ],
)
def test_validator_rejects_dangerous_or_invalid_gcode(line: str) -> None:
    profile = kp.load_profile()
    with pytest.raises(kp.GCodeSafetyError):
        kp.validate_gcode(f"G90\n{line}\nG1 X10 Y50 F1200\nM400\n", profile)


def test_raster_source_is_recognized_but_rejected(tmp_path: Path) -> None:
    source = tmp_path / "photo.jpg"
    source.write_bytes(b"not-an-image-needed-for-classification")
    with pytest.raises(kp.KobraPlotError, match="raster source recognized"):
        kp.classify_source(source)


def test_pdf_source_is_recognized_but_rejected(tmp_path: Path) -> None:
    source = tmp_path / "document.pdf"
    source.write_bytes(b"%PDF")
    with pytest.raises(kp.KobraPlotError, match="PDF source recognized"):
        kp.classify_source(source)


def test_prepare_curved_svg_end_to_end(tmp_path: Path) -> None:
    job = tmp_path / "curve-job"
    report = kp.prepare_file(SAMPLE, job)

    assert report["source"]["kind"] == "svg"
    assert report["geometry"]["polylines"] >= 1
    assert report["geometry"]["points"] > 2
    assert report["safety"]["valid"] is True
    assert report["execution"]["allowed"] is False

    gcode = (job / "output.gcode").read_text(encoding="utf-8")
    for forbidden in ("G28", "G91", "M104", "M109", "M140", "M190", " E"):
        assert forbidden not in gcode

    assert (job / "normalized.svg").is_file()
    assert (job / "preview.svg").is_file()
    assert (job / "report.json").is_file()


def test_prepare_text_file_end_to_end(tmp_path: Path) -> None:
    source = tmp_path / "message.txt"
    source.write_text("MILEGO DNIA\n", encoding="utf-8")
    job = tmp_path / "text-job"

    report = kp.prepare_file(source, job)
    assert report["source"]["kind"] == "text"
    assert report["geometry"]["polylines"] >= 1
    assert report["safety"]["valid"] is True
    assert report["execution"]["allowed"] is False
