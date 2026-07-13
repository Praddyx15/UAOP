"""Gap-detection test (TELEMETRY_ENGINE.md §5, EVENT_FLOW.md §3).

Publishes app-level sequence 1,2,3,5 (deliberately skipping 4) and asserts a
consumer correctly identifies the discontinuity from the delivered payloads'
own sequence field — the same "the record says where it isn't" mechanism
TELEMETRY_ENGINE.md §5 describes, proven against a real delivered stream
rather than asserted against a fixture.

The durable consumer is created BEFORE publishing, not after — TELEMETRY is
provisioned with interest-based retention (EVENT_FLOW.md §2), meaning a
message with no registered consumer interest at publish time is eligible for
immediate removal. In real operation this is a non-issue (telemetry-engine's
durable consumer is already running before any vehicle publishes); a test
that publishes first and subscribes after was found — by actually running
it — to lose every message it just sent, a genuine ordering bug in the test
itself rather than in the schema or the stream config.
"""

import pytest

from conftest import unique_suffix


def find_gaps(sequences: list[int]) -> list[tuple[int, int]]:
    gaps = []
    for prev, curr in zip(sequences, sequences[1:]):
        if curr != prev + 1:
            gaps.append((prev, curr))
    return gaps


@pytest.mark.asyncio
async def test_consumer_detects_a_skipped_app_level_sequence(jetstream):
    vehicle = f"veh-gaptest-{unique_suffix()}"
    subject = f"uaop.telemetry.v1.{vehicle}.position"
    durable = f"gap-test-{unique_suffix()}"

    sub = await jetstream.pull_subscribe(subject, durable=durable, stream="TELEMETRY")

    published = [1, 2, 3, 5]  # 4 is deliberately never published
    for seq in published:
        await jetstream.publish(subject, str(seq).encode())

    msgs = await sub.fetch(len(published), timeout=10)
    for msg in msgs:
        await msg.ack()

    received = [int(msg.data.decode()) for msg in msgs]
    assert received == published  # delivery order matches publish order (per-subject FIFO)

    gaps = find_gaps(received)
    assert gaps == [(3, 5)]  # exactly the one gap, correctly located


@pytest.mark.asyncio
async def test_consumer_sees_no_gap_when_sequence_is_contiguous(jetstream):
    vehicle = f"veh-nogaptest-{unique_suffix()}"
    subject = f"uaop.telemetry.v1.{vehicle}.position"
    durable = f"nogap-test-{unique_suffix()}"

    sub = await jetstream.pull_subscribe(subject, durable=durable, stream="TELEMETRY")

    published = [1, 2, 3, 4]
    for seq in published:
        await jetstream.publish(subject, str(seq).encode())

    msgs = await sub.fetch(len(published), timeout=10)
    for msg in msgs:
        await msg.ack()

    received = [int(msg.data.decode()) for msg in msgs]
    assert find_gaps(received) == []
