"""Durable JetStream consumer for `uaop.event.v1.param.written` (EVENT_FLOW.md §2: EVENTS stream).

Needs the Python protobuf modules buf generates from api/proto (buf.gen.yaml →
gen/python, on PYTHONPATH). Started by app.py only when UAOP_NATS_URL is set.
"""

import asyncio
import logging
from typing import Any

from .applied_changes import RetryLater, handle_param_written
from .flightmd_client import FlightmdClient
from .store import ReportStore

log = logging.getLogger(__name__)

SUBJECT = "uaop.event.v1.param.written"
DURABLE = "flight-log-engine-param-written"


def decode(data: bytes) -> dict[str, Any]:
    from uaop.events.v1 import envelope_pb2, param_pb2

    env = envelope_pb2.EventEnvelope()
    env.ParseFromString(data)
    p = param_pb2.ParamWritten()
    p.ParseFromString(env.payload)
    # by hand rather than MessageToDict, which leaves out zero values (old_value 0.0)
    return {
        "event_id": env.event_id,
        "vehicle_id": env.vehicle_id,
        "actor": env.actor,
        "sim": env.sim,
        "payload": {
            "name": p.name,
            "old_value": p.old_value,
            "new_value": p.new_value,
            "operator_id": p.operator_id,
            "recommendation": {
                "flightmd_report_id": p.recommendation.flightmd_report_id,
                "airframe_label": p.recommendation.airframe_label,
                "finding_id": p.recommendation.finding_id,
            },
        },
    }


async def run(nats_url: str, client: FlightmdClient, store: ReportStore, counts: dict[str, int]) -> None:
    import nats

    nc = await nats.connect(nats_url)
    js = nc.jetstream()
    sub = await js.pull_subscribe(SUBJECT, durable=DURABLE, stream="EVENTS")
    try:
        while True:
            try:
                msgs = await sub.fetch(10, timeout=5)
            except nats.errors.TimeoutError:
                continue
            for msg in msgs:
                try:
                    event = decode(msg.data)
                except Exception:
                    log.exception("undecodable param.written message, terminated")
                    await msg.term()
                    continue
                try:
                    # urllib blocks; keep the event loop free for the HTTP routes
                    outcome = await asyncio.to_thread(handle_param_written, event, client, store)
                except RetryLater as exc:
                    log.warning("FlightMD hand-back deferred: %s", exc)
                    await msg.nak(delay=30)
                    continue
                counts[outcome] = counts.get(outcome, 0) + 1
                await msg.ack()
    finally:
        await nc.close()
