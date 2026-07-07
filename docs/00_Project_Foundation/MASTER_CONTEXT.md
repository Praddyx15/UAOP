# UAOP Master Context — Single Source of Truth

**Version 0.1.0 · 2026-07-03 · Owner: Founder / Chief Architect**
**Change control: modifications require an ADR in [DECISIONS.md.**

This document is the project constitution. Every other document, every prompt given to an AI coding agent, and every line of implementation must remain consistent with it. Where any other artifact conflicts with this one, this one wins.

---

## 1. Product definition

**UAOP (Unified Autonomy Operating Platform)** is a modular operational platform that sits **above** existing UAV autonomy ecosystems. It unifies:

- Ground Control Station (GCS)
- Telemetry acquisition, storage, and streaming
- MAVLink and ROS 2 integration
- Deep autopilot tuning with AI assistance
- Simulation (SITL, Gazebo, HIL) and digital twin
- AI diagnostics and predictive maintenance
- Fleet management and enterprise operations
- Regulatory compliance tooling (FAA / EASA / DGCA)

**Explicit non-goals:**
- UAOP is **not** an autopilot. It never runs the inner control loops. PX4 and ArduPilot remain authoritative for flight control.
- UAOP is **not** a fork of QGroundControl, Mission Planner, or any ecosystem project. Integration is by **adapter**, never by merge.
- UAOP does not claim certification. It is engineered so that a future certification effort is practical (see §8).

## 2. Target users

UAV startups and OEMs · robotics companies using ROS 2 · research labs · enterprise inspection/mapping operators · defense and government (future) · pilot training institutes (future).

## 3. Deployment philosophy

Three tiers, in order of delivery:

1. **Workstation** — developer/simulation deployment. `docker compose up` plus the Qt GCS. Phase 1.
2. **Edge Node** — the primary product. A rugged compute unit (x86_64 or Jetson-class ARM64) near flight operations running the full service stack. Must operate with zero internet connectivity. Real-time decisions never depend on cloud latency.
3. **Cloud** — fleet analytics, multi-org enterprise, long-term archive. Phase 4 only. Cloud is an *optional amplifier*, never a dependency.

**Air-gap mode is a first-class deployment configuration.** No Phase 1–3 component may take a dependency on an external SaaS (no Firebase, no Supabase, no AWS SDKs). Storage access goes through a `StorageProvider` abstraction so a cloud backend can be introduced in Phase 4 without touching consuming services.

## 4. Technology spine (mandatory)

| Layer | Technology | Rationale summary (full ADRs in DECISIONS.md) |
|---|---|---|
| Core services | C++17, Drogon | Deterministic latency for telemetry/MAVLink paths; MISRA-checkable; single toolchain with Qt |
| AI / simulation orchestration | Python 3.11, FastAPI | ML ecosystem (PyTorch, SciPy); orchestration is not latency-critical |
| Desktop GCS | Qt 6.8 LTS / QML (C++20 backend — ADR-0016) | Native performance for dense real-time UI; aerospace heritage; offline by construction. **See ADR-0002** — supersedes the earlier Next.js/Electron direction |
| Web dashboard | Next.js (Phase 4, cloud tier only) | Fleet/analytics views for enterprise; not on the critical operational path |
| Internal bus | NATS JetStream | Lightweight enough for edge; persistent streams; subject-based routing; at-least-once delivery |
| Vehicle link | MAVLink v2 | Ecosystem standard; message signing available |
| Robotics link | ROS 2 Humble (DDS) | Ecosystem standard; migration path to Jazzy tracked in RISK_REGISTER |
| Service RPC | gRPC + Protobuf | Typed contracts; the `.proto` files in `api/proto/` are the platform's data constitution |
| UI streaming | WebSocket | GCS and web consumers subscribe to telemetry streams |
| Metadata / compliance DB | PostgreSQL 15 | Append-only audit tables with SHA-256 hash chain |
| Telemetry DB | TimescaleDB | Hypertables to 1000 Hz IMU ingest, continuous aggregates |
| Cache / ephemeral state | Redis 7 | Last-known vehicle state, session data |
| Object storage | MinIO | ULogs, reports, recordings; S3 API keeps Phase 4 cloud swap trivial |
| Simulation | Gazebo (Harmonic — ADR-0003), PX4 SITL, ArduPilot SITL, AirSim-successor (risk-flagged) | |
| ML | PyTorch (LSTM anomaly), SciPy/scikit-learn (FFT, PID optimisation), ONNX for edge inference | |
| Containers | Docker (Compose for workstation, k3s for edge, Kubernetes for cloud) | |
| Monitoring | Prometheus + Grafana | |
| Maps | Interim: `QQuickImageProvider` + MBTiles/SQLite raster renderer (ADR-0020). Target: MapLibre Native (Qt) + offline tile packages, deferred pending a Qt-6.11-matching prebuilt or a dedicated source-build spike | Open source, zero API keys, offline-first |

## 5. Architecture principles (non-negotiable)

1. **Microservices with strict boundaries** — services communicate only via NATS subjects and gRPC contracts; no shared databases between service domains.
2. **Event-driven** — NATS JetStream is the spine; request/response (gRPC) is reserved for commands and queries, never for streaming.
3. **Adapter pattern at every ecosystem boundary** — MAVLink, ROS 2, DJI (if adopted — OQ-1), simulators. External projects are consumed, never forked.
4. **Plugin-first** — every engine exposes a plugin interface (see PLUGIN_SYSTEM.md). Sandboxed, signed.
5. **Edge-capable** — every Phase 1–3 service builds for x86_64 and ARM64 and runs within Edge Node resource budgets.
6. **Compliance-by-design** — audit logging, requirement traceability (`@req:` annotations), and configuration management are structural, not features.
7. **Dependency inversion** — services depend on interfaces (`StorageProvider`, `MapProvider`, `VehicleLink`), never on concrete backends.

## 6. Canonical data contracts

Defined once in `api/proto/`, versioned, and never broken without a major version:

- `VehicleTelemetry` — full-fidelity state (position/GPS incl. RTK, attitude, velocity, raw IMU, EKF2 innovations, battery incl. per-cell, per-motor ESC, RC input, link quality, GPS jamming/spoofing indicators, system health, flight state, Remote ID). Full field census in [TELEMETRY_ENGINE.md.
- `TelemetrySnapshot` — lightweight fleet view.
- `TelemetryDelta` — bandwidth-constrained links.

NATS subject taxonomy (versioned in the subject, `v1`):

```
uaop.telemetry.v1.<vehicle_id>.<category>     # position|attitude|battery|esc|health|...
uaop.event.v1.<domain>.<event>                # mission.uploaded, failsafe.triggered, geofence.breach
uaop.cmd.v1.<vehicle_id>.<command>            # arm, mode_change, rtl (audit-logged)
uaop.ai.v1.<vehicle_id>.<artifact>            # flight_report, tuning_recommendation
uaop.audit.v1                                 # hash-chained audit stream (JetStream, persistent)
```

## 7. Service census

| Service | Language | Phase | Repo location |
|---|---|---|---|
| mavlink-bridge | C++ | 1 | `middleware/mavlink-bridge/` |
| ros2-bridge | C++ | 2 | `middleware/ros2-bridge/` |
| telemetry-engine | C++ | 1 | `backend/services/telemetry-engine/` |
| mission-engine | C++ | 1 | `backend/services/mission-engine/` |
| parameter-engine | C++ | 1 | `backend/services/parameter-engine/` |
| vehicle-manager | C++ | 1 | `backend/services/vehicle-manager/` |
| remote-id | C++ | 1 | `backend/services/remote-id/` |
| flight-log-engine | C++ | 1 | `backend/services/flight-log-engine/` |
| compliance-engine | C++/Python | 1 | `backend/services/compliance-engine/` |
| ai-engine | Python | 2 | `backend/services/ai-engine/` (consumes `flightmd_core`) |
| simulation-engine | Python/C++ | 3 | `backend/services/simulation-engine/` |
| fleet-management | C++ | 4 | `backend/services/fleet-management/` |
| api-gateway | Node.js gRPC-Gateway | 1 | `backend/api-gateway/` (ADR-0004) |
| dji-service | TBD | **unscoped — OQ-1** | `backend/services/dji-service/` (exists in repo, absent from constitution) |
| fpv-service | TBD | **unscoped — OQ-1** | `backend/services/fpv-service/` (exists in repo, absent from constitution) |

## 8. Compliance posture

- Process framework: **DO-178C-aligned** development (plans: PSAC, SDP, SVP, SCMP, SQA Plan; SAS at completion). Note for reviewers: UAOP is ground software, for which **DO-278A** is the formal analog; we apply DO-178C processes as the constitution mandates and record the DO-278A mapping in COMPLIANCE.md.
- System-level: **ARP4754A** development assurance concepts; **ARP4761** safety assessment (FHA at platform level).
- Tooling: **DO-330** awareness — any UAOP tool whose output enters a certification artifact (e.g., RTM generator) is a qualification candidate.
- Assurance levels: **DAL C-equivalent rigor** for flight-influencing modules (mavlink-bridge, vehicle-manager fail-safe logic, mission-engine upload/verify, remote-id); **DAL D-equivalent** for UI, analytics, logging.
- Coding: MISRA C++ on flight-influencing C++ code; no dynamic allocation or exceptions on real-time paths (`Result<T,E>` pattern); Cppcheck + clang-tidy in CI from the first commit.
- Traceability: `@req: UAOP-HLR-xxx` / `@req: UAOP-LLR-xxx-yy` / `@test: UAOP-TC-xxx-Z` annotations; RTM auto-generated by `compliance/tools/rtm-generator`.
- Audit: append-only PostgreSQL, SHA-256 hash chain. Logged events: PARAM_WRITE, MISSION_UPLOAD, COMMAND_SENT, FAILSAFE_TRIGGER, ARM, GEOFENCE_BREACH, REMOTE_ID_CYCLE, AI_RECOMMENDATION_{SHOWN,APPLIED,DISMISSED}.
- Regulatory scope: FAA (Part 107/89, ASTM F3411-22a, ASTM F3548-21/LAANC), EASA (2019/945+947, JARUS SORA v2.5, U-Space 2021/664–666), DGCA (Drone Rules 2021, Digital Sky, DAM). Full mapping in COMPLIANCE.md.
- **AI is advisory only.** Every recommendation carries a confidence score and model version; the operator decision (apply/dismiss) is audit-logged.

## 9. Development phases (summary — authoritative detail in ROADMAP.md)

- **Phase 1 — Core GCS foundation:** MAVLink bridge, telemetry, mission, parameters, vehicle manager, Remote ID, flight logs, compliance skeleton, Qt GCS core views, Docker Compose, CI with MISRA gate. Exit: SITL round-trip validation gate passes.
- **Phase 2 — Tuning + ROS 2 + AI diagnostics:** advanced tuning engine, ai-engine (FFT/LSTM/PID optimiser via `flightmd_core`), ros2-bridge, SORA module, BVLOS monitoring.
- **Phase 3 — Simulation + digital twin + training:** simulation orchestration, scenario/failure-injection library, certification test runner, digital twin, training scoring.
- **Phase 4 — Enterprise + SDK + certification readiness:** fleet management, multi-tenant RBAC, plugin SDK + marketplace, full UTM integrations, security hardening (mTLS/FIPS/HSM), DO-178C document package, cloud tier.

## 10. Related project: FlightMD

FlightMD ([github.com/Praddyx15/FlightMD](https://github.com/Praddyx15/FlightMD), MIT, separate repo — same author) is a **real, live, independently shipped product** (flightmd.vercel.app / flightmd-api.onrender.com), not a hypothetical. Its `flightmd_core` pip package is a fully deterministic (zero ML) rule-based flight-log analyser — PX4 ULog, ArduPilot `.bin`, and MAVLink `.tlog`, auto-detected — validated against 50 real-world logs across 11 vehicle types. It becomes ~30% of UAOP's ai-engine (AI_ENGINE.md §2.1).

Integration contract: `FlightMDReport` schema **v1.5** (corrected from the originally assumed v1.0 — the schema evolves independently of UAOP and every integration checks this field). Mode A (package import, offline, air-gap-safe, **no `AIEnhancer` passed by default**) is the default inside UAOP; Mode B (API call) is not used by UAOP. `flightmd_core` must never grow web dependencies. **UAOP imports only `flightmd_core`** — FlightMD's own web API, frontend, and fleet-operations features (maintenance tracking, webhooks, trends) are out of scope for UAOP and remain FlightMD's own product surface; see ADR-0018.

## 11. AI-assisted development workflow

Development is founder-led, with AI tooling used to accelerate drafting, review, and documentation where it helps. Every change — however drafted — is bound by this document, CODING_STANDARDS.md, and the phase scope in ROADMAP.md, and passes the same review and CI gates. A `progress.md` per phase records installation progress, dependency issues, integration status, review findings, benchmarks, and remaining work.

## 12. Open questions (OQ) — explicitly unresolved, do not assume

| ID | Question | Blocking | Where tracked |
|---|---|---|---|
| OQ-1 | `dji-service/` and `fpv-service/` exist in the repo but in no planning document. Is DJI (MSDK/Cloud API) and analog/digital FPV support in scope? If so, which phase? | Phase scoping only | DECISIONS.md §Open |
| ~~OQ-2~~ | RESOLVED 2026-07-04 (R2/F16): `middleware/mavlink-bridge/` created; separate-service boundary holds. | — | DECISIONS.md §R2 |
| ~~OQ-3~~ | RESOLVED 2026-07-07 (R6/ADR-0020): interim MBTiles raster renderer shipped; MapLibre Native Qt deferred (Qt version mismatch — see ADR-0020). | — | DECISIONS.md §R6 |
| OQ-4 | Platform licensing model (open-core vs proprietary) — affects SDK and marketplace design. | Phase 4 | BUSINESS_MODEL.md |
| OQ-5 | Business model pricing figures are proposals, not founder-validated. | None | BUSINESS_MODEL.md |
| OQ-6 | GCS emergency direct-connect: embed a minimal MAVLink client in the GCS for situational awareness + EMERGENCY commands if the edge node dies mid-flight? (Review R1/F9.) | Phase 2 planning | DECISIONS.md §Open |
| OQ-7 | UI plugin isolation model: declarative-only panels vs out-of-process rendering vs signed-trust acceptance. (Review R1/F10.) | Phase 4 planning | DECISIONS.md §Open |

## 13. Public positioning constraints

Public communications may reference ROS 2, Gazebo, AirSim, MAVLink, PX4, ArduPilot. Implementation details (bus technology, service topology, compliance tooling, UI design) are confidential. Job-application/LinkedIn usage: brief mention demonstrating systems thinking; UAOP is always described as a personal project in architecture phase, never as deployed.
