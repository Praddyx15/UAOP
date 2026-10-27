"""Report store for the sidecar: one JSON file per analysed flight under the data dir.

TODO(M5.2): move to the flight-log-engine's Postgres library tables and MinIO when
the C++ capture side lands; this keeps the same get/list/save surface so the
HTTP routes don't change.
"""

import json
import os
import re
from pathlib import Path
from typing import Any

# FlightMD report ids are uuid4 strings; event ids are UUIDv7. Both end up in file names.
_SAFE_ID = re.compile(r"^[A-Za-z0-9-]{1,64}$")


def safe_id(value: str) -> bool:
    return bool(_SAFE_ID.match(value or ""))


class ReportStore:
    def __init__(self, data_dir: Path) -> None:
        self.reports = data_dir / "reports"
        self.forwarded = data_dir / "forwarded"
        self.reports.mkdir(parents=True, exist_ok=True)
        self.forwarded.mkdir(parents=True, exist_ok=True)

    def _write(self, path: Path, data: dict[str, Any]) -> None:
        tmp = path.with_name(path.name + ".tmp")
        tmp.write_text(json.dumps(data, separators=(",", ":")), encoding="utf-8")
        os.replace(tmp, path)

    def save(self, view: dict[str, Any], flightmd_report: dict[str, Any]) -> None:
        report_id = view["summary"]["report_id"]
        if not safe_id(report_id):
            raise ValueError(f"unexpected report id {report_id!r}")
        # the raw FlightMD report is kept too: it is the evidence the view was cut from
        self._write(self.reports / f"{report_id}.json", {"view": view, "flightmd": flightmd_report})

    def get(self, report_id: str) -> dict[str, Any] | None:
        if not safe_id(report_id):
            return None
        try:
            stored = json.loads((self.reports / f"{report_id}.json").read_text(encoding="utf-8"))
        except FileNotFoundError:
            return None
        return stored["view"]

    def list(self, vehicle_id: str = "") -> list[dict[str, Any]]:
        out = []
        for p in self.reports.glob("*.json"):
            summary = json.loads(p.read_text(encoding="utf-8"))["view"]["summary"]
            if not vehicle_id or summary.get("vehicle_id") == vehicle_id:
                out.append(summary)
        # todo index file once a node holds more than a few hundred flights
        out.sort(key=lambda s: s.get("analysed_at", ""), reverse=True)
        return out

    def was_forwarded(self, event_id: str) -> bool:
        return (self.forwarded / event_id).exists()

    def mark_forwarded(self, event_id: str, outcome: str) -> None:
        self._write(self.forwarded / event_id, {"outcome": outcome})
