import json

import pytest

from flight_log_analysis.flightmd_client import FlightmdClient, FlightmdError

CHANGE = [{"name": "MC_ROLLRATE_P", "from_value": 0.18, "to_value": 0.15}]


def test_posts_the_change_with_the_airframe_token(flightmd):
    server = flightmd()
    client = FlightmdClient(server.url, {"x500 a": "tok-1"})
    out = client.record_changes("x500 a", "rep-1", CHANGE, note="applied by UAOP operator op-3")

    assert out["id"] == 7
    req = server.requests[0]
    assert req["path"] == "/airframe/x500%20a/changes"
    assert req["token"] == "tok-1"
    assert json.loads(req["body"]) == {
        "report_id": "rep-1",
        "changes": CHANGE,
        "applied_by": "uaop",
        "note": "applied by UAOP operator op-3",
    }


def test_missing_token_is_a_config_error_not_a_request(flightmd):
    server = flightmd()
    client = FlightmdClient(server.url, {})
    with pytest.raises(FlightmdError) as err:
        client.record_changes("x500", "rep-1", CHANGE)
    assert err.value.transient is False
    assert server.requests == []


@pytest.mark.parametrize("status,transient", [(401, False), (404, False), (422, False), (429, True), (503, True)])
def test_http_errors_say_whether_to_retry(flightmd, status, transient):
    server = flightmd(status=status)
    client = FlightmdClient(server.url, {"x500": "tok"})
    with pytest.raises(FlightmdError) as err:
        client.record_changes("x500", "rep-1", CHANGE)
    assert err.value.status == status
    assert err.value.transient is transient


def test_redirects_are_not_followed(flightmd):
    elsewhere = flightmd()
    server = flightmd(status=307, location=f"{elsewhere.url}/airframe/x500/changes")
    client = FlightmdClient(server.url, {"x500": "tok"})
    with pytest.raises(FlightmdError) as err:
        client.record_changes("x500", "rep-1", CHANGE)
    assert err.value.status == 307
    assert elsewhere.requests == []


def test_unreachable_server_is_transient():
    client = FlightmdClient("http://127.0.0.1:9", {"x500": "tok"}, timeout_s=2)
    with pytest.raises(FlightmdError) as err:
        client.record_changes("x500", "rep-1", CHANGE)
    assert err.value.transient is True


def test_disabled_without_a_url():
    assert FlightmdClient("", {}).enabled is False
