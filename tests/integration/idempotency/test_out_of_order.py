"""Out-of-order test (EVENT_FLOW.md §3: "Ordering: guaranteed per subject
only. Consumers needing cross-category order... order by the envelope's
sequence").

Publishes app-level sequence 3,1,2 in that literal wire order and asserts
delivery arrives in *publish* order, not *sorted* order — proving why the
envelope-sequence-ordering rule exists at all: NATS guarantees per-subject
FIFO delivery, never logical ordering. A consumer that assumes delivery
order is logical order is the bug this test exists to catch.

The durable consumer is created BEFORE publishing — see test_gap_detection.py
for why: TELEMETRY's interest-based retention (EVENT_FLOW.md §2) discards a
message immediately if no consumer has registered interest in it yet.
"""

import pytest

from conftest import unique_suffix


@pytest.mark.asyncio
async def test_delivery_order_matches_publish_order_not_logical_sequence(jetstream):
    vehicle = f"veh-ooo-{unique_suffix()}"
    subject = f"uaop.telemetry.v1.{vehicle}.attitude"
    durable = f"ooo-test-{unique_suffix()}"

    sub = await jetstream.pull_subscribe(subject, durable=durable, stream="TELEMETRY")

    publish_order = [3, 1, 2]  # logically out of order on purpose
    for seq in publish_order:
        await jetstream.publish(subject, str(seq).encode())

    msgs = await sub.fetch(len(publish_order), timeout=10)
    for msg in msgs:
        await msg.ack()

    received = [int(msg.data.decode()) for msg in msgs]

    assert received == publish_order  # FIFO by publish time, NOT sorted
    assert received != sorted(publish_order)  # the whole point: it is not logically ordered


@pytest.mark.asyncio
async def test_a_consumer_that_sorts_by_app_sequence_recovers_logical_order(jetstream):
    vehicle = f"veh-ooo-sort-{unique_suffix()}"
    subject = f"uaop.telemetry.v1.{vehicle}.attitude"
    durable = f"ooo-sort-test-{unique_suffix()}"

    sub = await jetstream.pull_subscribe(subject, durable=durable, stream="TELEMETRY")

    for seq in [3, 1, 2]:
        await jetstream.publish(subject, str(seq).encode())

    msgs = await sub.fetch(3, timeout=10)
    for msg in msgs:
        await msg.ack()

    received = [int(msg.data.decode()) for msg in msgs]
    # This is the fix EVENT_FLOW.md §3 prescribes: sort by the envelope's own
    # sequence field before treating delivery order as logical order.
    assert sorted(received) == [1, 2, 3]
