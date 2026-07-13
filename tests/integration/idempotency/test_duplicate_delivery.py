"""Duplicate-delivery test (TESTING.md §2 rule 3, EVENT_FLOW.md §3).

Publishes the same Nats-Msg-Id twice within the stream's duplicate window
(2 minutes, per the M1.5 provisioning script) and asserts JetStream's own
PubAck reports the second publish as a duplicate — the transport-level half
of "consumers dedup on it" (the consumer-side half, on a UUIDv7 event_id, is
services/common's DedupSet, unit-tested in dedup_jetstream_test.cpp).
"""

import pytest

from conftest import unique_suffix


@pytest.mark.asyncio
async def test_same_msg_id_published_twice_is_flagged_duplicate(jetstream):
    subject = f"uaop.event.v1.vehicle.idempotency_test_{unique_suffix()}"
    msg_id = f"evt-{unique_suffix()}"

    first_ack = await jetstream.publish(subject, b"first-delivery", headers={"Nats-Msg-Id": msg_id})
    second_ack = await jetstream.publish(subject, b"second-delivery-same-id", headers={"Nats-Msg-Id": msg_id})

    assert first_ack.duplicate is not True
    assert second_ack.duplicate is True
    # JetStream keeps the FIRST message's content on a dedup hit — the
    # duplicate publish never actually lands as new data.
    assert second_ack.seq == first_ack.seq


@pytest.mark.asyncio
async def test_different_msg_ids_are_not_deduped(jetstream):
    subject = f"uaop.event.v1.vehicle.idempotency_test_{unique_suffix()}"

    first_ack = await jetstream.publish(subject, b"payload-a", headers={"Nats-Msg-Id": f"evt-{unique_suffix()}"})
    second_ack = await jetstream.publish(subject, b"payload-b", headers={"Nats-Msg-Id": f"evt-{unique_suffix()}"})

    assert second_ack.duplicate is not True
    assert second_ack.seq != first_ack.seq
