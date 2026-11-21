# System architecture and allocation

UAOP-SA, issue 1, 2026-11-21. The items of the system, what exists of each,
and which system requirements each carries. The design intent is in
`docs/02_System_Architecture/SYSTEM_ARCHITECTURE.md` and
`docs/03_Software_Architecture/MICROSERVICES.md`; this file adds the item
names used in ALLOCATION and the as-found state.

## 1. Context

```
 pilot --- station (gcs) --- gateway --- bus (NATS JetStream) --- services --- vehicle-link --- radio --- autopilot
                                 |             |                      |
                              gRPC to       storage              (planned)
                              services   (PostgreSQL/TimescaleDB,
                                          Redis, MinIO)
```

Outside the software: the autopilot, the radios, the pilot's independent
control, and procedures.

## 2. Items

| Item (ALLOCATION name) | What it is | Where | State as found |
|---|---|---|---|
| `gcs` | Qt 6 / QML desktop station | `frontend/qt-desktop-gcs/` | Shell, panels, offline map and post-flight panel built. Vehicle state comes from a local generator, not from the gateway |
| `gateway` | REST and WebSocket front, gRPC client, token check | `backend/api-gateway/` | Built (v0): command forwarding, stream relay, development login |
| `common` | C++ runtime library: result type, config, logger, metrics, health, sequence, dedup, bus port | `backend/services/common/` | Built and unit-tested; bus adapter is in-memory only |
| `contracts` | Protobuf messages and services; subject and stream definitions | `api/proto/`, `tools/provision-streams.sh` | Built |
| `vehicle-link` | MAVLink bridge to the autopilot | Planned (`middleware/mavlink-bridge/` in the design) | Not in the tree |
| `vehicle-manager` | Vehicle state, command admission, authority | Planned | Not in the tree |
| `telemetry-engine` | Telemetry normalising and persistence | Planned | Not in the tree |
| `mission-engine` | Mission and geofence transfer | Planned | Not in the tree |
| `parameter-engine` | Parameter read, checked write | `backend/services/parameter-engine/` | Data files and one header only |
| `compliance-engine` | Audit chain writer, verifier, evidence bundle | Planned | Not in the tree |
| `flight-log` | Log capture; post-flight analysis sidecar | `backend/services/flight-log-engine/analysis-sidecar/` | Sidecar built; capture planned |
| `bus` | NATS JetStream, five streams | `infrastructure/docker/compose/compose.yaml`, `tools/provision-streams.sh` | Configured |
| `storage` | PostgreSQL with TimescaleDB, Redis, MinIO | `infrastructure/postgres/migrations/` | Ten migrations; Redis used by the gateway denylist |
| `tools` | Build, CI guards, trace matrix generator, service template | `tools/`, `compliance/tools/`, `.github/workflows/` | Built |
| `autopilot` | PX4 or ArduPilot (external) | - | External |
| `procedure` | Operator procedures (external) | - | External |

## 3. Allocation of system requirements

| Item | System requirements |
|---|---|
| `gcs` | SYS-003, 004, 005, 008, 010, 018, 023, 028 |
| `gateway` | SYS-007, 008, 009, 010, 011, 012, 013, 017, 018 |
| `common` | SYS-024, 027, 029 |
| `contracts` | SYS-005, 006, 009, 010, 024, 026 |
| `vehicle-link` | SYS-001, 003, 016 |
| `vehicle-manager` | SYS-002, 010, 011, 013, 014 |
| `telemetry-engine` | SYS-030 |
| `mission-engine` | SYS-020 |
| `parameter-engine` | SYS-019 |
| `compliance-engine` | SYS-014, 022, 028 |
| `flight-log` | SYS-017, 018, 021, 022, 023 |
| `bus` | SYS-006, 014, 016, 018, 024 |
| `storage` | SYS-015, 018, 030 |
| `tools` | SYS-026 |
| `autopilot`, `procedure` | SYS-025 |

## 4. The two critical paths

**Command path (planned end to end):** station, gateway REST route, gRPC
`VehicleCommandService`, vehicle manager (authority, idempotency, audit
event stored), command subject, vehicle link, MAVLink command, autopilot
acknowledgement, result event, gateway, station. Built today: the gateway
route and the contract. See `docs/lifecycle/04-safety/fmea.md` section 1.

**Telemetry path (planned end to end):** autopilot, vehicle link, telemetry
subjects, stream, gateway WebSocket relay, station display. Built today: the
contract, the stream definition and the relay. See the FMEA, section 2.

Interfaces are in `icd/`.
