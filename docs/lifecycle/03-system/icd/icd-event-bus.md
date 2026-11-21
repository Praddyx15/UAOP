# ICD: event bus (NATS JetStream)

Issue 1, 2026-11-21. Between all services. Sources: `tools/provision-streams.sh`,
`api/proto/uaop/events/v1/envelope.proto`,
`api/proto/uaop/telemetry/v1/`, `docs/02_System_Architecture/EVENT_FLOW.md`.

## 1. Subjects and streams

| Stream | Subjects | Retention as provisioned |
|---|---|---|
| TELEMETRY | `uaop.telemetry.v1.>` (`<vehicle_id>.<category>`) | Interest-based, 24 h, 10 GB, file storage |
| EVENTS | `uaop.event.v1.>` (`<domain>.<event>`) | Limits, 30 d, 5 GB |
| COMMANDS | `uaop.cmd.v1.>` (`<vehicle_id>.<command>`) | Work queue, 1 GB |
| AI | `uaop.ai.v1.>` | Limits, 30 d, 2 GB |
| AUDIT | Audit intake (see the script for subject and limits) | Persistent; not truncated before the chain is written |

## 2. Event envelope

Every message on `uaop.event.v1.*` is an `EventEnvelope`: `event_id`
(UUIDv7), `schema` (name and version of the payload), `sequence` (per source,
increasing), `occurred_at`, `source_service`, `vehicle_id` (empty for
platform events), `correlation_id`, `actor`, `sim` (simulation provenance,
never dropped), `payload` (bytes).

## 3. Telemetry messages

`VehicleTelemetry` (full state), `TelemetrySnapshot` (fleet view) and
`TelemetryDelta` (constrained links) in `api/proto/uaop/telemetry/v1/`. The
field list is the contract; it is not repeated here.

## 4. Delivery rules

At least once. Consumers discard duplicates by `event_id` and detect gaps by
`sequence` (`backend/services/common/include/uaop/common/dedup.h`,
`sequence.h`; `tests/integration/idempotency/`). Only parameter-written
events have a consumer today (the analysis sidecar, durable name
`flight-log-engine-param-written`).

## 5. Differences and open points

- No C++ client for a real NATS server exists; services have an in-memory
  adapter only.
- The AUDIT stream does not yet have the always-synced storage that the
  record-before-action rule needs (carried gap in
  `docs/07_Implementation_Guides/IMPLEMENTATION_PLAN.md`, section 0).
- No publisher exists for telemetry or command subjects.
- Permissions per subject (for example no publish on `uaop.cmd.>` for
  observer sessions) are designed, not configured.
