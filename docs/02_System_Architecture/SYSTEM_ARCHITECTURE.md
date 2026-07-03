# SYSTEM_ARCHITECTURE

**Version 0.1.0 · 2026-07-03 · C4 level 1–2 (context + containers). Component detail: SOFTWARE_ARCHITECTURE.md, MICROSERVICES.md.**

## 1. System context (C4-1)

UAOP is a ground-segment platform. Everything airborne stays in the flight stack's assurance boundary; UAOP observes, plans, configures, and records.

```mermaid
graph TB
    subgraph External Actors
        OP[Operator P1]
        ENG[Integration Engineer P2]
        CO[Compliance Officer P3]
    end
    subgraph Air Segment
        FC[PX4 / ArduPilot Flight Controller]
        CC[Companion Computer - ROS 2]
        SNS[Payloads / Sensors]
    end
    subgraph "UAOP (this system)"
        EDGE[UAOP Edge Node]
        GCS[Qt/QML Desktop GCS]
    end
    subgraph Regulatory / External Systems
        USS[USS / UTM - LAANC, U-Space, Digital Sky]
        TILES[Offline tile packages - prepared off-site]
    end
    subgraph Optional - Phase 4
        CLOUD[UAOP Cloud - fleet, analytics, archive]
    end

    OP --> GCS
    ENG --> GCS
    CO --> GCS
    GCS <-->|gRPC / WebSocket| EDGE
    FC <-->|MAVLink v2 - serial/UDP| EDGE
    CC <-->|ROS 2 DDS| EDGE
    SNS -->|RTSP/WebRTC video| EDGE
    EDGE <-->|HTTPS - when connected| USS
    TILES -.->|imported offline| EDGE
    EDGE <-.->|store-and-forward sync| CLOUD
```

**Boundary rules:**
- The **flight controller is authoritative** for all control. UAOP issues MAVLink commands (arm, mode, mission) that the autopilot may refuse; UAOP never bypasses autopilot safety logic.
- The **USS link is optional at runtime**. Air-gap deployments run with regulatory functions in record-and-defer mode.
- The **cloud link is optional always** (UAOP-NFR-001, UAOP-HLR-072).

## 2. Container view (C4-2) — one Edge Node

```mermaid
graph TB
    subgraph "Edge Node (k3s / Docker)"
        subgraph Middleware
            MAV[mavlink-bridge C++]
            ROS[ros2-bridge C++ - Ph2]
        end
        subgraph "Core Engines (C++/Drogon)"
            TEL[telemetry-engine]
            MIS[mission-engine]
            PAR[parameter-engine]
            VEH[vehicle-manager]
            RID[remote-id]
            FLE[flight-log-engine]
            CMP[compliance-engine]
        end
        subgraph "Python Services"
            AI[ai-engine - Ph2]
            SIM[simulation-engine - Ph3]
        end
        GW[api-gateway - Node gRPC-Gateway]
        NATS[(NATS JetStream)]
        PG[(PostgreSQL + TimescaleDB)]
        RD[(Redis)]
        MIO[(MinIO)]
    end
    GCS[Qt/QML GCS] <-->|gRPC cmd/query| GW
    GCS <-->|WebSocket streams| GW
    MAV <--> NATS
    ROS <--> NATS
    TEL <--> NATS
    MIS <--> NATS
    PAR <--> NATS
    VEH <--> NATS
    RID <--> NATS
    FLE <--> NATS
    CMP <--> NATS
    AI <--> NATS
    SIM <--> NATS
    GW <--> NATS
    TEL --> PG
    CMP --> PG
    FLE --> MIO
    VEH --> RD
```

**Reading the diagram:** the bus is the only many-to-many surface. Databases are service-private (telemetry-engine owns TimescaleDB hypertables; compliance-engine owns audit tables; no cross-service SQL — MICROSERVICES.md enforces ownership).

## 3. Segment responsibilities

| Segment | Owns | Explicitly does not own |
|---|---|---|
| Flight stack (PX4/ArduPilot) | Control loops, EKF, failsafe execution, mode logic | — |
| Companion (ROS 2) | Perception, offboard autonomy behaviors | Flight-critical failsafe |
| **UAOP Edge Node** | Ingest, supervision, mission/parameter management, recording, compliance, AI advisories, simulation | Control loops; autonomous command origination |
| **UAOP GCS** | Presentation, operator intent capture, local interaction state | Business logic (thin client over gateway APIs) |
| UAOP Cloud (Ph4) | Fleet aggregate views, long-term archive, org administration | Anything a flight depends on |

## 4. Key end-to-end sequences

### 4.1 Telemetry (steady state)

```mermaid
sequenceDiagram
    participant FC as Flight Controller
    participant MB as mavlink-bridge
    participant NJ as NATS JetStream
    participant TE as telemetry-engine
    participant TS as TimescaleDB
    participant GW as api-gateway
    participant UI as Qt GCS
    FC->>MB: MAVLink v2 (ATTITUDE, GPS_RAW_INT, SYS_STATUS...)
    MB->>MB: parse, validate, map to VehicleTelemetry proto
    MB->>NJ: publish uaop.telemetry.v1.{veh}.{category}
    NJ->>TE: consume (durable)
    TE->>TS: batched hypertable insert
    NJ->>GW: consume (rate-limited fan-out)
    GW->>UI: WebSocket frame (subscribed categories)
```

### 4.2 Mission upload (command path with verification — UAOP-HLR-010/012)

```mermaid
sequenceDiagram
    participant UI as Qt GCS
    participant GW as api-gateway
    participant ME as mission-engine
    participant MB as mavlink-bridge
    participant FC as Flight Controller
    participant CE as compliance-engine
    UI->>GW: gRPC UploadMission(plan)
    GW->>ME: validate + hash plan
    ME->>MB: gRPC SendMission(items)
    MB->>FC: MISSION_COUNT / MISSION_ITEM_INT sequence
    FC-->>MB: MISSION_REQUEST / MISSION_ACK
    MB-->>ME: upload result
    ME->>MB: download mission for byte-verify
    MB->>FC: MISSION_REQUEST_LIST
    FC-->>MB: mission items
    ME->>ME: compare round-trip vs plan hash
    ME->>CE: event uaop.event.v1.mission.uploaded (hash, operator)
    ME-->>UI: verified / mismatch (never silent success)
```

### 4.3 Failsafe surfacing (vehicle-originated)

```mermaid
sequenceDiagram
    participant FC as Flight Controller
    participant MB as mavlink-bridge
    participant VM as vehicle-manager
    participant CE as compliance-engine
    participant UI as Qt GCS
    FC->>MB: HEARTBEAT/STATUSTEXT/SYS_STATUS (failsafe bits)
    MB->>VM: uaop.telemetry.v1.{veh}.health
    VM->>VM: state machine -> EMERGENCY
    VM->>CE: uaop.event.v1.failsafe.triggered (audit)
    VM->>UI: uaop.event.v1.vehicle.state_changed
    Note over UI: Master caution per UX_GUIDELINES - one acknowledgeable alert, no popup storm
```

## 5. Trust zones

```mermaid
graph LR
    subgraph Z0[Zone 0 - Vehicle links]
        A[MAVLink serial/UDP, RTSP]
    end
    subgraph Z1[Zone 1 - Edge internal]
        B[NATS, services, DBs - mTLS Ph4, loopback/netns Ph1]
    end
    subgraph Z2[Zone 2 - Operator LAN]
        C[GCS <-> gateway - TLS + JWT]
    end
    subgraph Z3[Zone 3 - WAN optional]
        D[USS APIs, cloud sync - TLS, outbound-only]
    end
    A --> B --> C
    B --> D
```

Zone 0 is the least trustworthy despite being "our" vehicle: MAVLink is unauthenticated by default (signing optional), so the bridge treats every frame as untrusted input (bounds-check, rate-limit, never crash on malformed frames). Details: SECURITY.md.

## 6. Architecture invariants (violations are defects)

1. No service other than mavlink-bridge speaks MAVLink; no service other than ros2-bridge speaks DDS.
2. No service reads another service's database.
3. ai-engine cannot publish on `uaop.cmd.>` (ADR-0012, enforced by NATS permissions).
4. Every vehicle-bound command traverses vehicle-manager's authority check and is audit-logged before transmission.
5. The GCS contains no business logic that would make two GCS instances disagree given the same edge state.
6. Loss of any Zone 3 connection changes **nothing** about Zones 0–2 behavior except sync/USS status indicators.
