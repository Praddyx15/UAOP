import json
import shutil
from pathlib import Path

import pytest
from fastapi.testclient import TestClient

from flight_log_analysis import config
from flight_log_analysis.analysis import uaop_view
from flight_log_analysis.app import create_app

REPO = Path(__file__).resolve().parents[5]
SAMPLE_LOG = REPO / "tests" / "data" / "sample_logs" / "sample_flawed.ulg"


@pytest.fixture
def client(cfg):
    with TestClient(create_app(cfg)) as c:
        yield c


def test_health_and_metrics(client):
    assert client.get("/healthz").json() == {"status": "ok"}
    assert "uaop_fle_analyses_total" in client.get("/metrics").text


def test_reports_list_and_get(client, report):
    view = uaop_view(report, vehicle_id="veh-1")
    client.app.state.store.save(view, report)

    listed = client.get("/v1/flight-reports").json()
    assert [r["report_id"] for r in listed["reports"]] == [report["report_id"]]
    assert client.get("/v1/flight-reports", params={"vehicle_id": "veh-2"}).json()["reports"] == []

    got = client.get(f"/v1/flight-reports/{report['report_id']}").json()["report"]
    assert got["summary"]["vehicle_id"] == "veh-1"
    assert len(got["findings"]) == 2


def test_unknown_report_is_404(client):
    assert client.get("/v1/flight-reports/nope").status_code == 404
    assert client.get("/v1/flight-reports/..%2F..%2Fsecrets").status_code == 404


@pytest.mark.parametrize("name", ["../outside.ulg", "missing.ulg", "/etc/passwd", "sub/../../x.ulg"])
def test_analysis_only_reads_the_log_library(client, cfg, name):
    (cfg.log_dir.parent / "outside.ulg").write_bytes(b"ULog")
    assert client.post("/v1/analyses", json={"log_name": name}).status_code == 404


def test_analyses_a_sample_log(client, cfg):
    pytest.importorskip("flightmd_core")
    shutil.copy(SAMPLE_LOG, cfg.log_dir / "sample_flawed.ulg")

    resp = client.post("/v1/analyses", json={"log_name": "sample_flawed.ulg", "vehicle_id": "veh-1", "airframe_label": "x500"})
    assert resp.status_code == 201
    view = resp.json()
    assert view["summary"]["file_name"] == "sample_flawed.ulg"
    assert view["summary"]["airframe_label"] == "x500"
    assert all(f["category"] != "ascent_profile" for f in view["findings"])

    stored = json.loads((cfg.data_dir / "reports" / f"{view['summary']['report_id']}.json").read_text(encoding="utf-8"))
    assert stored["flightmd"]["report_id"] == view["summary"]["report_id"]
    assert client.get("/v1/flight-reports").json()["reports"][0]["report_id"] == view["summary"]["report_id"]


def test_config_needs_existing_dirs(tmp_path):
    with pytest.raises(config.ConfigError):
        config.load({})
    with pytest.raises(config.ConfigError):
        config.load({"UAOP_FLIGHT_LOG__DATA_DIR": str(tmp_path / "nope"), "UAOP_FLIGHT_LOG__LOG_DIR": str(tmp_path)})


@pytest.mark.parametrize("url", ["ftp://flightmd.local", "http://user:pw@flightmd.local", "flightmd.local"])
def test_config_rejects_bad_flightmd_urls(tmp_path, url):
    env = {"UAOP_FLIGHT_LOG__DATA_DIR": str(tmp_path), "UAOP_FLIGHT_LOG__LOG_DIR": str(tmp_path), "UAOP_FLIGHTMD__URL": url}
    with pytest.raises(config.ConfigError):
        config.load(env)


def test_config_reads_the_tokens_file(tmp_path):
    tokens = tmp_path / "tokens.json"
    tokens.write_text('{"x500": "tok-1"}', encoding="utf-8")
    cfg = config.load({
        "UAOP_FLIGHT_LOG__DATA_DIR": str(tmp_path),
        "UAOP_FLIGHT_LOG__LOG_DIR": str(tmp_path),
        "UAOP_FLIGHTMD__URL": "http://flightmd.local:8000/",
        "UAOP_FLIGHTMD__TOKENS_FILE": str(tokens),
    })
    assert cfg.flightmd_url == "http://flightmd.local:8000"
    assert cfg.flightmd_tokens == {"x500": "tok-1"}
    assert "tok-1" not in repr(cfg)
