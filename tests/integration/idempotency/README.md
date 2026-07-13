# Idempotency test harness (M1.8)

TESTING.md §2 rule 3: "every NATS consumer gets a duplicate-delivery test and
an out-of-order test" — mandatory, not optional, per EVENT_FLOW.md §3's
at-least-once/per-subject-ordering-only semantics. These tests run against a
**real** NATS JetStream server (the workstation compose profile, with the
M1.5 streams already provisioned) — no NATS mock exists here, deliberately:
idempotency is exactly the kind of property a mock would fake past.

## Running

```bash
bash tools/setup.sh up          # if the compose stack isn't already running
pip install -r tests/integration/idempotency/requirements.txt
pytest tests/integration/idempotency/ -v
```

Tests use their own uniquely-suffixed subjects/durables per run (not the real
`veh-*` vehicle IDs), so they never collide with real traffic or with each
other on repeat runs.

## What each test actually proves

- **`test_duplicate_delivery.py`** — publishes the same `Nats-Msg-Id` twice
  within the stream's duplicate window and asserts JetStream's own `PubAck`
  reports the second as a duplicate — the transport-level half of
  EVENT_FLOW.md §3's "consumers dedup on it" guarantee (the other half,
  consumer-side dedup on a UUIDv7 `event_id`, is `services/common`'s
  `DedupSet`, unit-tested in M1.4).
- **`test_gap_detection.py`** — publishes app-level sequence 1,2,3,5
  (skipping 4 on purpose) and asserts a consumer correctly identifies the
  discontinuity — TELEMETRY_ENGINE.md §5's gap accounting, proven against a
  real delivered stream rather than asserted against a fixture.
- **`test_out_of_order.py`** — publishes app-level sequence 3,1,2 in that
  literal wire order and asserts delivery arrives in *publish* order, not
  *sorted* order — proving why EVENT_FLOW.md §3's "order by the envelope's
  sequence" rule exists: NATS guarantees per-subject FIFO, never logical
  ordering, so a consumer that skips this step is the bug.
