from datetime import UTC, datetime

from flight_log_analysis.analysis import uaop_view


def test_ascent_profile_findings_are_dropped(report):
    view = uaop_view(report)
    assert [f["id"] for f in view["findings"]] == ["a1", "b2"]
    assert view["summary"]["critical_count"] == 1
    assert view["summary"]["warning_count"] == 1


def test_params_come_from_the_report_or_its_param_changes(report):
    view = uaop_view(report)
    by_id = {f["id"]: f for f in view["findings"]}
    assert by_id["a1"]["params"] == ["MC_ROLLRATE_P", "MC_ROLLRATE_D"]
    # schema 1.6 finding without `params`: names taken from param_changes
    assert by_id["b2"]["params"] == ["BAT_V_LOAD_DROP"]


def test_times_fall_back_to_the_ms_fields(report):
    by_id = {f["id"]: f for f in uaop_view(report)["findings"]}
    assert by_id["a1"]["t_start_s"] == 40.0
    assert by_id["b2"]["t_start_s"] == 200.5
    assert by_id["b2"]["t_end_s"] is None


def test_summary_fields(report):
    when = datetime(2026, 10, 27, 9, 30, tzinfo=UTC)
    view = uaop_view(report, vehicle_id="veh-1", airframe_label="x500-a", analysed_at=when)
    s = view["summary"]
    assert s["report_id"] == report["report_id"]
    assert s["vehicle_id"] == "veh-1"
    assert s["airframe_label"] == "x500-a"
    assert s["analysed_at"] == "2026-10-27T09:30:00Z"
    assert s["overall_score"] == 71.5
    assert s["letter_grade"] == "C"
    assert view["parse_warnings"] == ["2 GPS samples had no fix and were skipped"]
    assert view["param_change_sheet"][0]["unit"] == ""


def test_not_assessed_report_has_no_score(report):
    report["overall_score"] = 0
    report["letter_grade"] = "-"
    report["score_label"] = "Not assessed"
    assert uaop_view(report)["summary"]["overall_score"] is None


def test_unknown_severity_reads_as_info(report):
    report["findings"][0]["severity"] = "catastrophic"
    assert uaop_view(report)["findings"][0]["severity"] == "info"
