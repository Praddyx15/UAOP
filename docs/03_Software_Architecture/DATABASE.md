# DATABASE

**Version 0.1.0 · 2026-07-03 · PostgreSQL 15 + TimescaleDB, Redis 7, MinIO — roles, schemas, sizing, and the ownership discipline that keeps microservices micro.**

## 1. Store-to-purpose mapping

| Store | Purpose | Owner discipline |
|---|---|---|
| PostgreSQL 15 | Relational domain state: missions, parameters versions, audit chain, compliance, findings, users, node config | **Schema-per-service**; a service's role can only touch its own schema (DATA_FLOW.md §5) |
| TimescaleDB (extension, same cluster) | Telemetry hypertables + continuous aggregates | telemetry-engine exclusively |
| Redis 7 | Ephemeral: last-known vehicle state, sessions, dedup TTL sets | Keyspace prefixes per service; nothing durable lives here — Redis loss must cost nothing but a warm-up |
| MinIO | Blobs: flight logs, reports, recordings, update bundles, export bundles | Bucket-per-purpose with per-service policies; content-addressed where the object is evidence |

One Postgres cluster per node (not per service): on edge hardware, N database processes would waste the resource budget; isolation is achieved by schema + role, which is the correct boundary for this scale. Revisit only if cloud-tier load ever demands physical splits — the schema discipline makes that split mechanical.

## 2. Schema census (Phase 1)

```
mission.plans / plan_versions / deployments / geofences / airspace_imports
param.metadata_packs / versions / pending_writes
audit.chain (append-only) / exports
compliance.checklist_templates / checklist_runs / signoffs
flightlog.entries (metadata; blobs in MinIO)
vehicle.registry / profiles
auth.users / roles / tokens
node.settings / deployment_records
telemetry.<category> hypertables + *_1s / *_1min continuous aggregates
```

## 3. The audit chain, physically

`audit.chain(seq BIGINT GENERATED ALWAYS AS IDENTITY, prev_hash BYTEA, event JSONB, hash BYTEA, ...)` — INSERT-only role for compliance-engine (`REVOKE UPDATE, DELETE, TRUNCATE`), trigger-guarded against sequence manipulation, monthly partitions, hash recomputation verifier as a scheduled job + on-demand (`uaop-node export-audit` verifies before packaging). Wall-clock is annotation; `seq` is order (HARDWARE_ARCHITECTURE.md §5 clock note).

## 4. Telemetry sizing math (drives HARDWARE_ARCHITECTURE storage specs)

Rough, deliberately conservative: full-rate envelope ≈ 500 bytes/sample compressed columnar. 1 vehicle: IMU 1000 Hz + remainder ~100 Hz ≈ small-MB/s → **~15–25 GB/day/vehicle** full-rate before compression tiers; Timescale native compression (segments > 24 h) typically 8–15×. The DATA_FLOW.md §4 ladder (72 h full → 90 d @ 1 Hz → summaries) keeps a single-vehicle node in low-hundreds-GB steady state — hence the 512 GB minimum on RC-3 and the days-until-watermark forecast in DEVOPS.md §6. These figures are Phase 1 benchmark items, not folklore: the ingest benchmark records real bytes/day and the doc gets corrected by measurement.

## 5. Migrations

Forward-only, per-schema, versioned in each service's repo directory, applied by migration jobs during update (EDGE_ARCHITECTURE.md §6 — pre-snapshot, health-gated, restore-on-fail). Rules: additive-first (two-step for destructive changes: deprecate + backfill, then drop a release later); every migration ships with a rollback note even though execution is forward-only (the note is for the human running a restore); migration version is a `doctor` check and a smoke-suite assertion.

## 6. Performance engineering notes

- Hypertable chunking: 1 h chunks per vehicle-category at high rate (keeps chunk indexes RAM-resident on RC-3); compression policy after 24 h; continuous aggregates refresh on write-completion windows, serving most UI history queries from aggregates (TELEMETRY_ENGINE.md §4 cost guards).
- Hot Postgres paths (audit insert, param version insert) are prepared-statement, connection-pooled (per-service pools, small — edge budget), and monitored for p99 (OBSERVABILITY.md).
- Redis: `maxmemory` with `allkeys-lru` on cache prefixes; dedup sets use TTL matching the JetStream redelivery horizon; last-known-state uses no eviction (tiny, bounded by vehicle count).
- MinIO on edge: single-node mode (erasure coding needs 4+ drives — false safety on one disk); integrity via content hashes verified on read (MICROSERVICES.md flight-log-engine).

## 7. Failure modes

| Failure | Effect | Design response |
|---|---|---|
| Postgres down | Audit chain append lags (CAUTION); commands stay available per ADR-0015; telemetry buffers in JetStream | Restart + WAL recovery; backfill drains buffers and chain append catches up; incident is visible, bounded, and lossless for persisted subjects |
| Timescale bloat / slow queries | History queries degrade | Aggregates + compression + cost guards; live path unaffected by architecture |
| Redis loss | Warm-up: PROVISIONAL vehicle states ~2 s (MICROSERVICES.md vehicle-manager), sessions re-auth | Accepted by design — nothing durable there |
| MinIO corruption | Evidence integrity threatened | Content-hash verification detects; restore from backup; capture blocks loudly when storage unhealthy (flight-log-engine) |
| Disk full | The platform's most likely field failure | Watermarks 80/90%, forecast surfacing, retention ladder, prioritized writes (audit > logs > telemetry history) |
