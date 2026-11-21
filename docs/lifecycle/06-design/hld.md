# High-level design, as built

UAOP-HLD, issue 1, 2026-11-21. What the software is today, item by item,
from reading the tree. The intended design is in
`docs/02_System_Architecture/` and `docs/03_Software_Architecture/` and is
not repeated; where the build differs, the difference is a problem report.

## 1. Shape

A set of services around a message bus, a gateway in front of them and a
desktop station, built from one repository with CMake (C++20), Node.js and
Python. Local infrastructure runs in Docker Compose: NATS 2.10 with
JetStream, PostgreSQL 15 with TimescaleDB, Redis 7, MinIO, and an optional
PX4 simulator profile.

## 2. Items as built

| Item | Language | Parts | Tests seen |
|---|---|---|---|
| Contracts (`api/proto/`) | Protobuf | Telemetry messages, event envelope, parameter event, error codes, six gateway services | Lint and breaking-change check in CI |
| Common library (`backend/services/common/`) | C++20 | `Result`, `Error`, layered configuration, JSON logger, metrics and health registries with an HTTP server, UUIDv7, sequence counters, deduplication set, bus port with an in-memory adapter | Seven unit test files |
| API gateway (`backend/api-gateway/`) | JavaScript (Node.js, Express, ws) | Token issue and check, revoked list in Redis, command forwarding over gRPC, WebSocket relay from the bus | Three test files, eleven tests |
| Analysis sidecar (`backend/services/flight-log-engine/analysis-sidecar/`) | Python 3.11 (FastAPI) | Analysis of a stored log with an external deterministic analyser package, report store in files, consumer of parameter-written events, optional hand-back client | Four test files |
| Parameter data (`backend/services/parameter-engine/`) | JSON, one header | Safe ranges, PX4 defaults for two versions, applied-recommendation type | None |
| Storage (`infrastructure/postgres/migrations/`) | SQL | Ten migrations: schemas and roles, mission, parameters, audit chain, compliance, flight log, vehicle, authentication, node, telemetry hypertables | Applied by `tools/db-migrate.sh`; no test seen |
| Desktop station (`frontend/qt-desktop-gcs/`) | C++20, QML (Qt 6) | Shell with panel host and workspace manager; views for telemetry, mission, compliance, defence, post-mission; offline map from a tile file; flight-reports model; `TelemetryController`, a local generator of vehicle state | Three unit test files (report parsing, tile provider, tile maths) |
| Service template (`tools/service-template/`) | C++20 | Ports-and-adapters skeleton with a link monitor as its example domain | One unit test file |
| Tools (`tools/`, `compliance/tools/`) | Python, shell | CI guards, theme-token generator, stream provisioning, migrations runner, trace matrix generator, static-analysis configuration | One unit test file for tokens |
| Integration harness (`tests/integration/idempotency/`) | Python | Duplicate delivery, gap detection, out-of-order delivery against the bus | Three test files |

Not in the tree: vehicle link, vehicle manager, telemetry engine, mission
engine, parameter engine logic, compliance engine, flight-log capture.
Two folders (`dji-service`, `fpv-service`) are frozen and empty by rule.

## 3. Data flows that work today

1. **Post-flight report.** Log file in the log library, sidecar analysis,
   report file, sidecar HTTP route, station post-mission panel.
2. **Applied-change hand-back.** Parameter-written event on the bus, sidecar
   consumer, ledger, optional call to an external analyser. There is no
   publisher of that event yet.
3. **Command forwarding.** REST request, gateway, gRPC call. There is no
   server; tests use a mock.
4. **Stream relay.** Bus subject, gateway, WebSocket frame. There is no
   telemetry publisher, and the station does not connect.

## 4. Data flows that do not exist

Vehicle to bus; bus to database; command to vehicle; audit event to chain;
station to gateway. The station's telemetry, status and arm, disarm and
abort actions are produced inside `TelemetryController` on a 100 ms timer
(PR-001, PR-002, PR-003).

## 5. Design rules in force

Contract before code; additive-only contracts; one schema and role per
service; errors as values in C++; units in names; no network dependency
outside the node. Sources: `CONTRIBUTING.md`,
`docs/08_Developer_Handbook/CODING_STANDARDS.md`.

## 6. Differences between design documents and build

See `docs/lifecycle/10-configuration/problem-reports.md`: PR-008 (routes and
ports), PR-009 (vehicle link directory), PR-013 (process frame and level
names), PR-014 (language standard).
