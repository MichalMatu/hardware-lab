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


def test_normalized_path_parser_rejects_unexpected_curves() -> None:
    with pytest.raises(kp.GeometryError, match="unsupported SVG path syntax"):
        kp.parse_linear_path("M 0 0 C 10 10 20 20 30 30")


def test_svg_preflight_rejects_silently_discarded_text(tmp_path: Path) -> None:
    source = tmp_path / "text.svg"
    source.write_text(
        '<svg xmlns="http://www.w3.org/2000/svg"><text x="1" y="2">hello</text></svg>',
        encoding="utf-8",
    )
    with pytest.raises(kp.GeometryError, match="<text>"):
        kp.preflight_svg_source(source)


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
    assert safety["draw_moves"] == 1
    assert safety["command_whitelist"] == ["G90", "G0", "G1", "M400"]
    for forbidden in ("G28", "G91", "M104", "M109", "M140", "M190", " E"):
        assert forbidden not in gcode


@pytest.mark.parametrize(
    "gcode",
    [
        "G90\nG28\nM400\n",
        "G90\nG91\nM400\n",
        "G90\nM104 S200\nM400\n",
        "G90\nM109 S200\nM400\n",
        "G90\nM140 S60\nM400\n",
        "G90\nM190 S60\nM400\n",
        "G90\nG0 Z6.12 F180\nG0 X10 Y50 F3000\nG0 Z3.12 F180\nG1 X20 Y60 E1 F1200\nG0 Z6.12 F180\nM400\n",
        "G90\nG0 Z6.12 F180\nG0 X219 Y50 F3000\nG0 Z3.12 F180\nG1 X20 Y60 F1200\nG0 Z6.12 F180\nM400\n",
        "G90\nG0 Z6.12 F180\nG0 X10 Y50 F3000\nG1 X20 Y60 F1200\nG0 Z6.12 F180\nM400\n",
        "G90\nG0 Z6.12 F180\nG0 X10 Y50 F3000\nG0 Z3.12 F180\nG0 X20 Y60 F3000\nG0 Z6.12 F180\nM400\n",
        "G90\nG0 Z6.12 F180\nG0 X10 Y50 F999\nG0 Z3.12 F180\nG1 X20 Y60 F1200\nG0 Z6.12 F180\nM400\n",
    ],
)
def test_validator_rejects_dangerous_or_invalid_gcode(gcode: str) -> None:
    with pytest.raises(kp.GCodeSafetyError):
        kp.validate_gcode(gcode, kp.load_profile())


def test_force_refuses_unmarked_directory(tmp_path: Path) -> None:
    source = tmp_path / "source.svg"
    source.write_text("<svg/>", encoding="utf-8")
    output = tmp_path / "existing"
    output.mkdir()
    (output / "keep.txt").write_text("keep", encoding="utf-8")

    with pytest.raises(kp.KobraPlotError, match="unmarked directory"):
        kp._prepare_output_dir(source.resolve(), output.resolve(), force=True)
    assert (output / "keep.txt").read_text(encoding="utf-8") == "keep"


def test_force_can_replace_only_marked_job_directory(tmp_path: Path) -> None:
    source = tmp_path / "source.svg"
    source.write_text("<svg/>", encoding="utf-8")
    output = tmp_path / "job"
    output.mkdir()
    (output / ".kobra-plot-job").write_text("kobra-plot-job-v1\n", encoding="utf-8")
    (output / "old.txt").write_text("old", encoding="utf-8")

    kp._prepare_output_dir(source.resolve(), output.resolve(), force=True)
    assert not (output / "old.txt").exists()
    assert (output / ".kobra-plot-job").read_text(encoding="utf-8") == "kobra-plot-job-v1\n"


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

    bounds = report["geometry"]["output_bounds"]
    assert 8 <= bounds["min_x"] <= bounds["max_x"] <= 218
    assert 41 <= bounds["min_y"] <= bounds["max_y"] <= 225

    gcode = (job / "output.gcode").read_text(encoding="utf-8")
    for forbidden in ("G28", "G91", "M104", "M109", "M140", "M190", " E"):
        assert forbidden not in gcode

    assert (job / ".kobra-plot-job").is_file()
    assert (job / "normalized.svg").is_file()
    assert (job / "preview.svg").is_file()
    assert (job / "report.json").is_file()
    assert report["artifacts"]["normalized_svg_sha256"]
    assert report["artifacts"]["preview_svg_sha256"]
    assert report["artifacts"]["gcode_sha256"]


def test_prepare_text_file_end_to_end(tmp_path: Path) -> None:
    source = tmp_path / "message.txt"
    source.write_text("MILEGO DNIA\n", encoding="utf-8")
    job = tmp_path / "text-job"

    report = kp.prepare_file(source, job)
    assert report["source"]["kind"] == "text"
    assert report["geometry"]["polylines"] >= 1
    assert report["safety"]["valid"] is True
    assert report["execution"]["allowed"] is False
