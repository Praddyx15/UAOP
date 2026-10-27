"""flightmd-core wrapper and the UAOP view of its report (log.proto FlightReport).

Mode A from ADR-0014/0018: the package is imported, no AIEnhancer is passed, so the
analysis makes no network call. Ascent-profile findings (rockets/HABs) are dropped
here at the boundary (Review R4/F26), not patched in flightmd-core.
"""

from datetime import UTC, datetime
from pathlib import Path
from typing import Any

SEVERITIES = ("critical", "warning", "info", "good")
DROPPED_CATEGORIES = {"ascent_profile"}


async def analyse(path: Path) -> dict[str, Any]:
    """Run flightmd-core on one log and return the report as plain JSON data.

    Raises ValueError for a log flightmd-core can't analyse (its own messages).
    """
    from flightmd_core import run_analysis  # heavy import (pandas, scipy): only when used

    report = await run_analysis(
        ulog_path=str(path),
        file_name=path.name,
        file_size=path.stat().st_size,
        ai_enhancer=None,
    )
    return report.model_dump(mode="json")


def _finding(f: dict[str, Any]) -> dict[str, Any]:
    params = f.get("params")
    if params is None:
        # reports before schema 1.7 only name the parameters inside param_changes
        params = [c["param_name"] for c in f.get("param_changes") or [] if c.get("param_name")]
    t_start = f.get("t_start_s")
    t_end = f.get("t_end_s")
    if t_start is None and f.get("timestamp_start_ms") is not None:
        t_start = f["timestamp_start_ms"] / 1000.0
    if t_end is None and f.get("timestamp_end_ms") is not None:
        t_end = f["timestamp_end_ms"] / 1000.0
    severity = f.get("severity")
    return {
        "id": f.get("id", ""),
        "category": f.get("category", ""),
        "severity": severity if severity in SEVERITIES else "info",
        "title": f.get("title", ""),
        "plain_english": f.get("plain_english", ""),
        "recommendation": f.get("recommendation", ""),
        "confidence": float(f.get("confidence") or 0.0),
        "t_start_s": t_start,
        "t_end_s": t_end,
        "params": list(params),
        "kind": f.get("kind") or "",
    }


def uaop_view(
    report: dict[str, Any],
    *,
    vehicle_id: str = "",
    airframe_label: str = "",
    analysed_at: datetime | None = None,
) -> dict[str, Any]:
    """FlightReport (log.proto) from a FlightMD report dict."""
    findings = [_finding(f) for f in report.get("findings") or [] if f.get("category") not in DROPPED_CATEGORIES]
    meta = report.get("metadata") or {}
    assessed = report.get("assessed_fraction")
    # FlightMD scores 0 with letter "-" when nothing was assessed; 0 would read as a crash
    score = report.get("overall_score")
    if report.get("letter_grade") == "-":
        score = None
    summary = {
        "report_id": report["report_id"],
        "log_id": "",
        "vehicle_id": vehicle_id,
        "airframe_label": airframe_label,
        "file_name": report.get("file_name", ""),
        "analysed_at": (analysed_at or datetime.now(UTC)).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "schema_version": str(report.get("schema_version", "")),
        "overall_score": score,
        "score_label": report.get("score_label", ""),
        "letter_grade": report.get("letter_grade", ""),
        "critical_count": sum(f["severity"] == "critical" for f in findings),
        "warning_count": sum(f["severity"] == "warning" for f in findings),
    }
    return {
        "summary": summary,
        "executive_summary": report.get("executive_summary", ""),
        "findings": findings,
        "param_change_sheet": [
            {
                "param_name": p.get("param_name", ""),
                "current_value": p.get("current_value"),
                "suggested_value": p.get("suggested_value"),
                "unit": p.get("unit") or "",
                "change_direction": p.get("change_direction", ""),
                "reason": p.get("reason", ""),
            }
            for p in report.get("param_change_sheet") or []
        ],
        "parse_warnings": list(meta.get("parse_warnings") or []),
        "assessed_fraction": assessed,
    }
