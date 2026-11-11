# TESTING.md §2 rule 3: every NATS consumer gets a duplicate-delivery and an
# out-of-order test. These drive the handler with decoded events; the JetStream side
# is the same pattern tests/integration/idempotency already covers.

import json

import pytest

from flight_log_analysis.applied_changes import RetryLater, handle_param_written
from flight_log_analysis.flightmd_client import FlightmdClient
from flight_log_analysis.store import ReportStore


def event(event_id, name="MC_ROLLRATE_P", old=0.18, new=0.15, report_id="rep-1", label="x500"):
    return {
        "event_id": event_id,
        "vehicle_id": "veh-1",
        "actor": "op-3",
        "payload": {
            "name": name,
            "old_value": old,
            "new_value": new,
            "operator_id": "op-3",
            "recommendation": {"flightmd_report_id": report_id, "airframe_label": label, "finding_id": "a1"},
        },
    }


@pytest.fixture
def store(cfg):
    return ReportStore(cfg.data_dir)


def test_recommended_write_is_forwarded(flightmd, store):
    server = flightmd()
    client = FlightmdClient(server.url, {"x500": "tok"})

    assert handle_param_written(event("0192e3b4-0000-7000-8000-000000000001"), client, store) == "forwarded"
    body = json.loads(server.requests[0]["body"])
    assert body["report_id"] == "rep-1"
    assert body["applied_by"] == "uaop"
    assert body["changes"] == [{"name": "MC_ROLLRATE_P", "from_value": 0.18, "to_value": 0.15}]
    assert "op-3" in body["note"] and "veh-1" in body["note"]


def test_duplicate_delivery_is_forwarded_once(flightmd, store):
    server = flightmd()
    client = FlightmdClient(server.url, {"x500": "tok"})
    e = event("0192e3b4-0000-7000-8000-000000000002")

    assert handle_param_written(e, client, store) == "forwarded"
    assert handle_param_written(e, client, store) == "duplicate"
    assert len(server.requests) == 1


def test_out_of_order_events_are_each_forwarded(flightmd, store):
    server = flightmd()
    client = FlightmdClient(server.url, {"x500": "tok"})
    first = event("0192e3b4-0000-7000-8000-000000000003", name="MC_ROLLRATE_P")
    second = event("0192e3b4-0000-7000-8000-000000000004", name="MC_ROLLRATE_D", old=0.003, new=0.0025)

    assert handle_param_written(second, client, store) == "forwarded"
    assert handle_param_written(first, client, store) == "forwarded"
    names = [json.loads(r["body"])["changes"][0]["name"] for r in server.requests]
    assert names == ["MC_ROLLRATE_D", "MC_ROLLRATE_P"]


def test_zero_old_value_is_sent_as_zero(flightmd, store):
    server = flightmd()
    client = FlightmdClient(server.url, {"x500": "tok"})
    handle_param_written(event("0192e3b4-0000-7000-8000-000000000005", old=0.0, new=1.0), client, store)
    assert json.loads(server.requests[0]["body"])["changes"][0]["from_value"] == 0.0


def test_plain_operator_write_is_not_forwarded(flightmd, store):
    server = flightmd()
    client = FlightmdClient(server.url, {"x500": "tok"})
    e = event("0192e3b4-0000-7000-8000-000000000006", report_id="", label="")
    assert handle_param_written(e, client, store) == "not_recommended"
    assert server.requests == []


def test_handback_off_without_flightmd_url(store):
    assert handle_param_written(event("0192e3b4-0000-7000-8000-000000000007"), FlightmdClient("", {}), store) == "disabled"


def test_bad_event_id_is_not_used_as_a_file_name(flightmd, store):
    server = flightmd()
    client = FlightmdClient(server.url, {"x500": "tok"})
    assert handle_param_written(event("../../etc/passwd"), client, store) == "invalid"
    assert server.requests == []


def test_flightmd_down_leaves_the_event_for_redelivery(flightmd, store):
    server = flightmd(status=503)
    client = FlightmdClient(server.url, {"x500": "tok"})
    e = event("0192e3b4-0000-7000-8000-000000000008")
    with pytest.raises(RetryLater):
        handle_param_written(e, client, store)
    assert not store.was_forwarded(e["event_id"])


def test_refused_change_is_recorded_and_not_retried(flightmd, store):
    server = flightmd(status=401)
    client = FlightmdClient(server.url, {"x500": "wrong"})
    e = event("0192e3b4-0000-7000-8000-000000000009")
    assert handle_param_written(e, client, store) == "refused"
    assert handle_param_written(e, client, store) == "duplicate"
    assert len(server.requests) == 1
