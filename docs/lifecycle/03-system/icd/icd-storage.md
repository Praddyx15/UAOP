# ICD: storage

Issue 1, 2026-11-21. Between services and the stores. Written from
`infrastructure/postgres/migrations/` and
`infrastructure/docker/compose/compose.yaml`. Design:
`docs/03_Software_Architecture/DATABASE.md`.

## 1. PostgreSQL 15 with TimescaleDB

One schema per service domain, one database role per service, no access
across domains.

| Schema | Holds | Migration |
|---|---|---|
| `mission` | Plans, versions (with content hash), deployments, geofences, airspace imports | 0002 |
| `param` | Metadata packs, versions, pending writes | 0003 |
| `audit` | `chain` (partitioned; `prev_hash`, `hash`), `exports` | 0004 |
| `compliance` | Checklist templates, runs, sign-offs | 0005 |
| `flightlog` | Entries (content hash, object pointer), reports | 0006 |
| `vehicle` | Registry, profiles | 0007 |
| `auth` | Users, roles, tokens | 0008 |
| `node` | Settings, deployment records | 0009 |
| `telemetry` | Hypertables by category (position, attitude and others), one-hour chunks | 0010 |

Audit chain rules in the database: an insert trigger refuses a row whose
`prev_hash` is not the current head (or a first row with a non-null
`prev_hash`); update, delete and truncate are revoked from the writer role.

## 2. Other stores

| Store | Use | State |
|---|---|---|
| Redis 7 | Revoked-token list for the gateway; last-known vehicle state (planned) | Denylist built |
| MinIO | Flight logs, reports, exports | Configured; no writer |
| Local files | Analysis sidecar reports and forwarded-event ledger; map tiles (MBTiles) for the station | Built |

## 3. Open points

- No service writes to PostgreSQL yet, so the schema is verified only by
  the migrations applying.
- The trigger takes the chain head by the highest sequence; behaviour under
  concurrent writers depends on the single-writer rule of the design and is
  to be analysed when the writer is built.
- The sidecar keeps reports in files, not in `flightlog.reports`.
