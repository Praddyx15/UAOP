# PRODUCT_REQUIREMENTS

**Version 0.1.0 · 2026-07-03 · Requirement IDs are permanent; never renumber.**

Requirement convention (feeds the RTM): `UAOP-HLR-nnn` high-level requirements below; low-level requirements (`UAOP-LLR-nnn-mm`) are derived per service in the engine documents and in code via `@req:` annotations. Verification method tags: **T** = test, **A** = analysis, **I** = inspection, **D** = demonstration.

## 1. Personas

| Persona | Cares about | Representative demand |
|---|---|---|
| **P1 Operator** (licensed pilot) | Situational awareness, failsafe clarity | "Show me everything that matters in one glance; never surprise me." |
| **P2 Integration engineer** (OEM/robotics) | Tuning depth, ROS 2 visibility, SITL parity | "Let me see MAVLink and ROS 2 state side by side." |
| **P3 Safety/compliance officer** | Audit trail, SORA/Part 107/Digital Sky records | "Give me the complete, tamper-evident record." |
| **P4 Fleet manager** (Phase 4) | Multi-vehicle status, utilisation, maintenance | "Which aircraft can fly today, and why not the others?" |
| **P5 Researcher/instructor** (Phase 3) | Reproducible scenarios, scoring, replay | "Re-run yesterday's failure against new parameters." |

## 2. Functional requirements

### Vehicle connectivity & state
- **UAOP-HLR-001 (T)** The platform SHALL ingest MAVLink v2 telemetry from PX4 and ArduPilot vehicles over serial, UDP, and TCP, at sustained 10 Hz full-state rate per vehicle, ≥50 Hz for attitude/IMU streams.
- **UAOP-HLR-002 (T)** The platform SHALL maintain a supervised state machine per vehicle (DISCONNECTED→CONNECTED→READY→ARMED→IN_FLIGHT→LANDING→EMERGENCY) with all transitions event-published and audit-logged.
- **UAOP-HLR-003 (T)** The platform SHALL support ≥5 simultaneous vehicles in Phase 1, ≥50 by Phase 3, 255 (MAVLink system-ID limit) by Phase 4.
- **UAOP-HLR-004 (T)** Loss of vehicle link SHALL be detected within 3 s (heartbeat timeout) and surfaced in UI within 1 s of detection.
- **UAOP-HLR-005 (T)** *(added by review R1/ADR-0017)* The platform SHALL maintain exactly one CONTROLLING session per vehicle with explicit handover (request/accept, supervisor-forced with reason, or automatic-to-UNCONTROLLED on session death); non-controlling command attempts SHALL be refused except EMERGENCY-class commands (RTL/LAND/revert-to-manual) from any authenticated PILOT-role session; all authority transitions SHALL be audit-logged.

### Mission
- **UAOP-HLR-010 (T)** The platform SHALL create, validate, upload, download, and byte-verify missions (waypoints, speeds, actions, DO_LAND_START, RTL) against PX4 and ArduPilot via the MAVLink mission microservice protocol, with retry and round-trip verification.
- **UAOP-HLR-011 (T)** The platform SHALL support inclusion/exclusion geofences (polygon and circle) with configurable breach action (RTL/LAND/LOITER/HOLD/REPORT); breach events are audit-logged.
- **UAOP-HLR-012 (A)** Every uploaded mission SHALL be content-hashed and the hash audit-logged (mission identity for compliance).

### Parameters & tuning
- **UAOP-HLR-020 (T)** The platform SHALL read the full parameter set, present it grouped and searchable, validate writes against type and safe-range metadata, and version every change (who/when/old/new).
- **UAOP-HLR-021 (T)** Parameter writes SHALL be confirmed via MAVLink ACK with read-back verification; unACKed writes are surfaced as failures, never assumed.
- **UAOP-HLR-022 (T)** The tuning engine SHALL provide live PID response visualisation (setpoint vs response vs error) and risk-classified write flow (LOW/MEDIUM/HIGH; HIGH requires explicit acknowledgment).
- **UAOP-HLR-023 (T)** AI tuning recommendations SHALL carry confidence scores, SHALL be constrained to the safe operating envelope, and SHALL require operator action to apply (ADR-0012).

### Telemetry, logging, analysis
- **UAOP-HLR-030 (T)** All telemetry SHALL be persisted to TimescaleDB at full received rate (design ceiling 1000 Hz IMU) and streamed to UI consumers over WebSocket at configurable rates.
- **UAOP-HLR-031 (T)** The platform SHALL import and analyse PX4 ULog and ArduPilot DataFlash logs via the 7-module analyser suite (oscillation, vibration, EKF, battery, GPS, parameter anomaly, motor/ESC) producing scored findings with plain-language explanations.
- **UAOP-HLR-032 (A)** Flight logs SHALL be hash-chained at capture for tamper evidence.

### Remote ID & compliance
- **UAOP-HLR-040 (T)** The platform SHALL implement ASTM F3411-22a Remote ID monitoring/management: broadcast state, mandatory field display, ≥1 Hz cycle verification, USS network status; broadcast cycles audit-logged.
- **UAOP-HLR-041 (D)** The platform SHALL provide jurisdiction-aware compliance tooling: FAA (Part 107/89, LAANC status), EASA (SORA SAIL calculator per JARUS v2.5, 24-OSO checklist, PDF report), DGCA (Digital Sky flight-plan state, DAM zones).
- **UAOP-HLR-042 (A)** The audit log SHALL be append-only with a SHA-256 hash chain; any gap or reorder SHALL be detectable by the export verifier.
- **UAOP-HLR-043 (T)** A 30-item categorised pre-flight checklist SHALL gate an AUTHORISE FLIGHT action; sign-off is audit-logged with operator identity.

### Simulation & digital twin (Phase 3)
- **UAOP-HLR-050 (T)** The platform SHALL launch/supervise PX4 SITL and ArduPilot SITL with Gazebo from the UI, including environment selection and failure injection (GPS loss, motor-out, compass jam, comms loss, battery sag, wind gust).
- **UAOP-HLR-051 (T)** The digital twin SHALL mirror live vehicle state with measured divergence display and +30 s trajectory prediction, and SHALL replay recorded flights.
- **UAOP-HLR-052 (D)** A certification test runner SHALL execute scripted scenario suites and emit signed PDF/JSON reports.

### ROS 2 (Phase 2)
- **UAOP-HLR-060 (T)** The platform SHALL browse ROS 2 nodes/topics, render the node graph, inspect live messages, manage lifecycle nodes, and display per-topic bandwidth/rate — without requiring RViz/rqt.
- **UAOP-HLR-061 (T)** MAVLink-derived and ROS 2-derived state SHALL be reconciled with end-to-end latency <50 ms, monitored and alarmed.

### Fleet & enterprise (Phase 4)
- **UAOP-HLR-070 (T)** Multi-organisation tenancy with RBAC (PLATFORM_ADMIN, ORG_ADMIN, FLEET_MANAGER, PILOT, OBSERVER, MAINTENANCE).
- **UAOP-HLR-071 (T)** Fleet dashboard: per-vehicle status, utilisation, maintenance state, with per-vehicle command authority respecting RBAC.
- **UAOP-HLR-072 (T)** Edge→cloud synchronisation SHALL be store-and-forward, resumable, and never required for flight operations.

### Extensibility
- **UAOP-HLR-080 (T)** Every engine SHALL expose a plugin interface; plugins are signed, sandboxed, and capability-scoped (PLUGIN_SYSTEM.md).

## 3. Non-functional requirements

| ID | Requirement | Verification |
|---|---|---|
| UAOP-NFR-001 | **Air-gap:** all Phase 1–3 functions operate with zero internet connectivity. | D — disconnect test each release |
| UAOP-NFR-002 | Telemetry glass-to-glass latency (vehicle→UI) ≤ 250 ms at P95 on reference edge hardware. | T — instrumented SITL bench |
| UAOP-NFR-003 | Edge Node steady-state budget (1 vehicle, full stack): ≤ 4 GB RAM, ≤ 50% of a Jetson Orin NX. | T — soak test |
| UAOP-NFR-004 | Any single service crash SHALL not take down the platform; supervised restart ≤ 5 s; UI degrades gracefully with explicit staleness indication. | T — fault injection |
| UAOP-NFR-005 | 24 h soak with simulated telemetry: zero message loss on JetStream-persisted subjects, zero unbounded memory growth. | T |
| UAOP-NFR-006 | Cold start (edge node power-on → operational UI) ≤ 120 s. | T |
| UAOP-NFR-007 | All builds reproducible on x86_64 and ARM64 from one CI pipeline. | I |
| UAOP-NFR-008 | Flight-influencing C++: MISRA-clean (zero critical), no dynamic allocation/exceptions on RT paths. | A — static analysis gate |
| UAOP-NFR-009 | Requirements traceability: 100% of flight-influencing code paths annotated; RTM generated in CI. | A |
| UAOP-NFR-010 | Security baseline per SECURITY.md: TLS on all non-loopback transport, JWT+RBAC on all APIs, signed plugins, encrypted at-rest secrets. | T/I |
| UAOP-NFR-011 | *(R1/F3)* GCS rendering: EFIS pair sustains 60 fps; all panels update at ≥ subscribed data rate with 5 vehicles across 3 displays on RC-1-class hardware; no unbounded UI memory growth over 8 h. | T |
| UAOP-NFR-012 | *(R1/F3)* Time discipline: all stored records ordered by per-source monotonic sequence; wall-clock traceable to NTP or GPS-disciplined source with drift annotation when neither is available. | A/T |

## 4. Out of scope (current)

Payload-specific tooling (gimbal/camera control beyond video display), photogrammetry processing, swarm *autonomy algorithms* (UI monitoring of formations is in scope; formation control is the flight stack's job), DJI/FPV protocol support pending OQ-1, iPad app, and any autopilot-firmware modification.

## 5. Acceptance philosophy

A requirement is done when: implemented → `@req:`-annotated → verified by its tagged method in CI or the phase validation gate → and demonstrated against SITL (or hardware where applicable). Phase gates in [ROADMAP.md bundle these into exit criteria.
