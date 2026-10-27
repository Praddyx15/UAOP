"""Tune, fly, verify: hand a parameter write that applied a FlightMD recommendation back
to FlightMD, so its next report of the airframe can say whether the change helped.

Input is a decoded `uaop.event.v1.param.written` event (events/v1/param.proto
ParamWritten inside the EventEnvelope). The audit record itself is that event, which
parameter-engine persisted before this ever runs (ADR-0015); this only tells FlightMD.
Delivery is at-least-once, so event ids already forwarded are skipped.
"""

import logging
from typing import Any

from .flightmd_client import FlightmdClient, FlightmdError
from .store import ReportStore, safe_id

log = logging.getLogger(__name__)


class RetryLater(Exception):
    """FlightMD is down or busy: leave the event un-acked so JetStream redelivers it."""


def handle_param_written(event: dict[str, Any], client: FlightmdClient, store: ReportStore) -> str:
    """Forward one param.written event; returns what happened (also the metrics label)."""
    payload = event.get("payload") or {}
    rec = payload.get("recommendation") or {}
    report_id = rec.get("flightmd_report_id") or ""
    label = rec.get("airframe_label") or ""
    if not report_id or not label:
        return "not_recommended"  # an ordinary operator write
    if not client.enabled:
        return "disabled"
    event_id = event.get("event_id") or ""
    if not safe_id(event_id):
        log.warning("param.written event without a usable event_id, not forwarded")
        return "invalid"
    if store.was_forwarded(event_id):
        return "duplicate"

    change = {"name": payload["name"], "from_value": payload["old_value"], "to_value": payload["new_value"]}
    note = f"applied by UAOP operator {event.get('actor') or payload.get('operator_id') or 'unknown'}"
    if event.get("vehicle_id"):
        note += f" on {event['vehicle_id']}"
    try:
        client.record_changes(label, report_id, [change], note=note)
    except FlightmdError as exc:
        if exc.transient:
            raise RetryLater(str(exc)) from exc
        # 4xx: wrong token, unknown airframe or report. Retrying won't fix it; the write
        # itself is already audited, so record the outcome and move on.
        log.warning("FlightMD refused change %s for %s: %s", event_id, label, exc)
        store.mark_forwarded(event_id, f"refused:{exc.status or 'config'}")
        return "refused"
    store.mark_forwarded(event_id, "forwarded")
    return "forwarded"
