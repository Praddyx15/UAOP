# ICD: gateway API (station and other clients)

Issue 1, 2026-11-21. Between clients (the station, scripts) and the gateway.
Written from `backend/api-gateway/src/`. The design is
`docs/03_Software_Architecture/API_SPECIFICATION.md`; differences are in
section 4.

## 1. REST (default port 8080)

| Route | Auth | Behaviour |
|---|---|---|
| `GET /healthz` | None | Liveness |
| `POST /auth/dev-login` | None | Body `{subject, role, org}`; returns a token for any valid role. Development stub (PR-007) |
| `POST /auth/logout` | Token | Meant to revoke the token until it expires; as wired it answers 401 (PR-015) |
| `POST /vehicles/{id}/commands` | Token | Body `{command, params}`; header `Idempotency-Key`. `202 {correlation_id}`; `400` for an unknown command; `502` when the service call fails |
| `GET /commands/{correlationId}` | Token | The command result; `502` when the service call fails |

Tokens: JSON Web Token, HS256, default life 3600 s, claims subject, role,
organisation and token id. Roles: PLATFORM_ADMIN, ORG_ADMIN, FLEET_MANAGER,
PILOT, OBSERVER, MAINTENANCE. Errors are `{error: {code, message}}` with the
codes of `api/proto/uaop/errors/v1/errors.proto`.

## 2. WebSocket (`/ws/v1`, same port)

Client to server: `{"op":"subscribe","streams":[...]}` where a stream is
`{"type":"telemetry","vehicle":V,"categories":[...],"rate_hz":N}` or
`{"type":"events","domains":[...]}`. A new subscribe replaces the previous
set.

Server to client:

| Frame | Meaning |
|---|---|
| `{stream, seq, ts, payload}` | One relayed message. `seq` counts from 1 per stream per connection; `ts` is the gateway's send time; `payload` is the bus message, base64 |
| `{op:"gap", stream, dropped}` | Messages dropped by rate shaping since the last frame; sent just before the next relayed frame |
| `{op:"heartbeat", ts}` | Every 5 s by default |
| `{op:"error", message}` | Invalid JSON or unknown operation |

## 3. The station today

The station does not use this interface for vehicle state or commands. It
reads flight reports from the analysis sidecar over HTTP and map tiles from
a local file. Its vehicle state and its arm, disarm and abort actions are
local (PR-001, PR-003).

## 4. Differences from the design

- Routes are served at the root, not under `/api/v1`; the WebSocket is on
  the HTTP port, not on 8081, and the `wsPort` setting is unused (PR-008).
- The WebSocket has no authentication and does not validate the vehicle,
  category or domain strings (PR-004).
- No route checks the role (PR-005).
- `ts` is not the source time (PR-006).
- TLS is designed, not configured.
