import copy
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer

import pytest

from flight_log_analysis import config

REPORT_17 = {
    "report_id": "5d1c2a7e-3f7b-4e8a-9c41-0b6f2d9e8a10",
    "schema_version": "1.7",
    "overall_score": 71.5,
    "score_label": "Caution",
    "letter_grade": "C",
    "executive_summary": "Roll oscillation at 18 Hz during the hover.",
    "file_name": "flight_042.ulg",
    "file_size_bytes": 1234,
    "processing_time_ms": 900,
    "assessed_fraction": 0.9,
    "metadata": {
        "duration_seconds": 312.0,
        "armed_windows": [[12.0, 300.5]],
        "parse_warnings": ["2 GPS samples had no fix and were skipped"],
        "planning_brief_sha256": None,
    },
    "findings": [
        {
            "id": "a1",
            "category": "oscillation",
            "severity": "critical",
            "title": "Roll oscillation",
            "technical_summary": "18 Hz, 4 deg p-p",
            "plain_english": "The roll axis shakes.",
            "recommendation": "Lower MC_ROLLRATE_P.",
            "confidence": 0.9,
            "timestamp_start_ms": 40000,
            "timestamp_end_ms": 52000,
            "t_start_s": 40.0,
            "t_end_s": 52.0,
            "param_changes": [],
            "params": ["MC_ROLLRATE_P", "MC_ROLLRATE_D"],
            "kind": "oscillation.roll",
        },
        {
            "id": "b2",
            "category": "battery",
            "severity": "warning",
            "title": "Voltage sag",
            "technical_summary": "",
            "plain_english": "",
            "recommendation": "",
            "confidence": 0.6,
            "timestamp_start_ms": 200500,
            "timestamp_end_ms": None,
            "param_changes": [
                {"param_name": "BAT_V_LOAD_DROP", "current_value": 0.3, "suggested_value": 0.4,
                 "unit": "V", "change_direction": "increase", "reason": "sag"},
            ],
        },
        {
            "id": "c3",
            "category": "ascent_profile",
            "severity": "warning",
            "title": "Apogee early",
            "technical_summary": "",
            "plain_english": "",
            "recommendation": "",
            "confidence": 0.5,
        },
    ],
    "param_change_sheet": [
        {"param_name": "MC_ROLLRATE_P", "current_value": 0.18, "suggested_value": 0.15,
         "unit": None, "change_direction": "decrease", "reason": "oscillation"},
    ],
    "analyser_results": [],
}


@pytest.fixture
def report():
    return copy.deepcopy(REPORT_17)


@pytest.fixture
def cfg(tmp_path):
    data = tmp_path / "data"
    logs = tmp_path / "logs"
    data.mkdir()
    logs.mkdir()
    return config.load({"UAOP_FLIGHT_LOG__DATA_DIR": str(data), "UAOP_FLIGHT_LOG__LOG_DIR": str(logs)})


class FakeFlightmd:
    """A real HTTP server on loopback standing in for FlightMD's /airframe/{label}/changes."""

    def __init__(self, status=201, location=None):
        self.requests = []
        fake = self

        class Handler(BaseHTTPRequestHandler):
            def do_POST(self):
                length = int(self.headers.get("Content-Length", 0))
                fake.requests.append({
                    "path": self.path,
                    "token": self.headers.get("X-Airframe-Token"),
                    "body": self.rfile.read(length).decode("utf-8"),
                })
                self.send_response(fake.status)
                if fake.location:
                    self.send_header("Location", fake.location)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                if fake.status == 201:
                    self.wfile.write(b'{"id": 7, "created_at": "2026-10-27T10:00:00Z"}')

            def log_message(self, *args):
                pass

        self.status = status
        self.location = location
        self.server = HTTPServer(("127.0.0.1", 0), Handler)
        self.url = f"http://127.0.0.1:{self.server.server_port}"
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()

    def close(self):
        self.server.shutdown()
        self.server.server_close()


@pytest.fixture
def flightmd():
    servers = []

    def start(**kwargs):
        s = FakeFlightmd(**kwargs)
        servers.append(s)
        return s

    yield start
    for s in servers:
        s.close()
