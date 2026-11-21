# Low-level design: API gateway

UAOP-LLD-GW, issue 1, 2026-11-21. As built, from `backend/api-gateway/src/`.
Requirements: `docs/lifecycle/05-requirements/llr-gateway.sdoc`. Interface:
`docs/lifecycle/03-system/icd/icd-gateway-api.md`.

## 1. Modules

| File | Role | LLR |
|---|---|---|
| `index.js` | Starts Redis and bus connections, builds the HTTP application, installs handlers in order, attaches the WebSocket server to the same HTTP server, returns a `close()` | LLR-GW-008 |
| `config.js` | Settings from environment variables with defaults (HTTP port 8080, bus and Redis addresses, gRPC target, token secret and life, heartbeat interval) | LLR-GW-006, 009, 015 |
| `auth/jwt.js` | Issue and verify tokens (HS256) | LLR-GW-009 |
| `auth/denylist.js` | Revoked-token list in Redis with expiry | LLR-GW-010 |
| `auth/middleware.js` | `authenticate` (token check) and `requireRole` (role check; defined, not installed anywhere) | LLR-GW-007 |
| `auth/roles.js` | The six role names | LLR-GW-009 |
| `routes/auth.js` | Development login and logout | LLR-GW-008 |
| `routes/health.js` | `GET /healthz` | - |
| `routes/commands.js` | Command table, submit and result routes | LLR-GW-001 to 005 |
| `grpc/client.js` | Loads the contracts, caches one client per service and target, unencrypted channel | LLR-GW-006 |
| `streams/websocket.js` | `ClientSession` per connection, `StreamState` per stream | LLR-GW-011 to 016 |

## 2. Command forwarding

```
POST /vehicles/:id/commands
  body.command --lookup--> CommandKind      (miss: 400)
  request = {vehicleId, command, params|{}, idempotencyKey|''}
  client.submitCommand(request)             (error: 502)
  --> 202 {correlation_id}
```

The route holds no state. It sends no call metadata, so the service learns
nothing about the caller, and it sets no deadline on the call. It never
decides the outcome of a command; that belongs to the service and arrives by
`GET /commands/:correlationId` or as an event on the stream.

## 3. Stream relay

One `ClientSession` per socket holds the bus subscriptions and a map of
`StreamState` (key, sequence, dropped count, minimum interval, time of last
relay). Each subscription is drained by its own asynchronous loop:

```
for each bus message on the subscription:
  if shaping says not yet: dropped += 1; continue
  if dropped > 0: send gap frame; dropped = 0
  seq += 1; send {stream, seq, ts: now, payload: base64}
```

Properties worth knowing when reading the safety assessment:

- Order within a stream is the order of arrival from the bus subscription.
- `seq` is local to the connection; it is not the source sequence.
- `ts` is the gateway's send time.
- Shaping uses the gateway's wall clock; a clock step changes the rate
  briefly.
- A slow client is not detected: frames are handed to the socket library
  without a check of its send buffer.
- The subscriptions are plain bus subscriptions, not durable consumers: a
  message published while the gateway is disconnected is not relayed later.

## 4. Errors and limits

| Case | Behaviour |
|---|---|
| Bus unreachable at start | Start fails and the process exits with an error |
| Redis unreachable at start | Not checked at start in the code read (to confirm by test) |
| Redis unreachable during a token check | Not handled in the code read; the request fails (to confirm by test) |
| gRPC target down | 502 per request |
| Malformed WebSocket message | Error frame; connection kept |

## 5. Known gaps (not fixed in this phase)

PR-004, PR-005, PR-006, PR-007, PR-008, PR-010, PR-015 in
`docs/lifecycle/10-configuration/problem-reports.md`.
