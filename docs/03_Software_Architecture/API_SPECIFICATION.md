# API_SPECIFICATION

**Version 0.1.0 · 2026-07-03 · Client-facing API through api-gateway. Internal service gRPC lives with each service; canonical schemas in `api/proto/`.**

## 1. Surfaces and conventions

| Surface | Use | Port (default) |
|---|---|---|
| REST/JSON `https://<node>:8080/api/v1` | Queries, CRUD, exports | 8080 |
| gRPC `<node>:50051` | Commands, transactional operations, SDK clients | 50051 |
| WebSocket `wss://<node>:8081/ws/v1` | Telemetry/event streams | 8081 |

Conventions: URI-versioned (`/api/v1`); JSON is proto3 JSON mapping of the same messages (one schema, two encodings); UUIDv7 ids; RFC 3339 UTC timestamps; errors use a single envelope:

```json
{ "error": { "code": "VEHICLE_NOT_READY", "message": "...", "correlation_id": "...", "details": {} } }
```

Error codes are a registry in `api/proto/errors.proto` — services may not invent strings ad hoc.

## 2. Authentication & authorization

- `Authorization: Bearer <JWT>` on every call (locally-issued tokens Phase 1–3; OIDC federation Phase 4).
- Claims carry role (RBAC six-role model) + org (Phase 4). Gateway enforces route-level policy; vehicle-manager re-checks command authority (defense in depth — the gateway is not the only wall).
- WebSocket authenticates at upgrade; tokens refresh in-band without stream teardown.

## 3. REST resource map (Phase 1 scope)

```
GET    /vehicles                          list + summary state
GET    /vehicles/{id}                     full state
GET    /vehicles/{id}/telemetry/history   ?fields=&from=&to=&rate=   (downsampled server-side)
GET    /vehicles/{id}/parameters          ?group=&search=
PUT    /vehicles/{id}/parameters/{name}   validated write (202 + correlation_id)
GET    /vehicles/{id}/parameters/versions
GET    /missions                          library
POST   /missions                          create (validation errors itemised)
POST   /missions/{id}/upload?vehicle=     202 + correlation_id; result via event/poll
GET    /missions/{id}/verify?vehicle=     round-trip check state
GET    /geofences · POST /geofences · POST /geofences/{id}/upload
GET    /logs · GET /logs/{id} · GET /logs/{id}/export?format=ulog|csv|json
POST   /logs/{id}/analyse                 (Ph2 → ai-engine)
GET    /reports/{id} · GET /reports/{id}/pdf
GET    /audit?from=&to=&domain=           chain-verified export: /audit/export
GET    /checklists/preflight · POST /checklists/preflight/{run}/sign
GET    /remoteid/{vehicle}                broadcast state + field values
GET    /node/status                       degradation ladder, versions, disk watermarks
```

## 4. Command semantics (the part that must never be sloppy)

Commands are **asynchronous with mandatory resolution**:

1. `POST /vehicles/{id}/commands` `{ "command": "ARM", "params": {} }` → `202 { "correlation_id": ... }`
2. Resolution arrives as `vehicle.command_result` on the event stream and via `GET /commands/{correlation_id}`.
3. Terminal states: `ACCEPTED` (FC acknowledged) · `REJECTED` (FC or authority refused, with reason) · `TIMEOUT` (link uncertainty — **UI must render as "state unknown, verify"**, never as failure-therefore-safe).

Idempotency: clients send `Idempotency-Key`; replays return the original correlation, preventing double-arm on retry.

## 5. WebSocket protocol

Client → server:
```json
{ "op": "subscribe", "streams": [
    { "type": "telemetry", "vehicle": "veh-01", "categories": ["position","attitude","battery"], "rate_hz": 10 },
    { "type": "events", "domains": ["vehicle","failsafe","mission"] } ] }
```
Server → client frames carry `stream`, `seq`, `ts`, and payload. Contract guarantees: per-stream ordering; explicit `{"op":"gap", "dropped": n}` when shaping drops frames (the UI staleness machinery keys off this); heartbeat every 5 s; resubscription is cheap and stateless.

## 6. gRPC services (client-facing, mirrored by REST above)

`VehicleCommandService`, `MissionService`, `ParameterService`, `TelemetryQueryService`, `LogService`, `ComplianceService` — definitions in `api/proto/gateway/v1/`. gRPC is the primary surface for the SDK and scripting; REST exists for humans, curl, and future web tier.

## 7. Versioning & deprecation policy

Additive-only within v1 (fields, endpoints, event types). Breaking → `/api/v2` beside v1 for ≥ one minor release cycle with `Deprecation` headers and gateway metrics on v1 usage. The GCS↔gateway handshake (SOFTWARE_ARCHITECTURE.md §7) pins compatibility explicitly, so "mystery blank panel" version skew cannot occur.

## 8. Rate and abuse controls

Per-token rate classes (query vs command vs export); command endpoints additionally throttled per-vehicle (an accidental `while true; do arm; done` should exhaust its budget, not the airframe's patience); export endpoints run as async jobs with progress polling to keep the gateway non-blocking.
