# DECISIONS — Architecture Decision Records

**Version 0.1.0 · 2026-07-03**

Format: lightweight ADR (Status · Context · Decision · Alternatives rejected · Consequences). New technology enters the stack only through an ADR. Statuses: `ACCEPTED`, `PROPOSED`, `SUPERSEDED(by)`, `OPEN`.

---

## ADR-0001 — UAOP composes ecosystems via adapters; it never forks them
**Status: ACCEPTED (founding decision)**
**Context:** UAV tooling is fragmented across PX4/ArduPilot/MAVLink/ROS 2/Gazebo. Two ways to unify: absorb (fork) or compose (adapt).
**Decision:** All ecosystem integration happens through adapter services (`middleware/`, `integrations/`) speaking the projects' own public protocols. Upstream versions are pinned; patches go upstream.
**Alternatives rejected:** Forking QGC as a UI base (inherits an unfamiliar 15-year codebase and its release politics); building a new autopilot (multi-year, zero flight heritage, kills the "sits above" thesis).
**Consequences:** We absorb upstream API churn at adapter boundaries only. We can never offer features requiring autopilot internals — acceptable, that's PX4/ArduPilot's job.

## ADR-0002 — Desktop GCS is Qt 6 / QML; the Next.js/Electron stack is superseded
**Status: ACCEPTED · resolves the constitution conflict**
**Context:** The two founding documents disagree. The Knowledge Base (older, "mandatory" table) specifies Next.js 14 + Electron + SwiftUI. The Master Context (newer, "single source of truth") and the repository skeleton (`frontend/qt-desktop-gcs/`) specify Qt/QML. This is the single largest ambiguity in the founding documents and must be resolved explicitly, not silently.
**Decision:** The operational desktop GCS is **Qt 6 / QML with a C++ backend**. The Next.js web dashboard survives as the **Phase 4 cloud tier** (fleet/analytics views only — never the primary flight interface). SwiftUI iPad cockpit is deferred indefinitely. The 24-tab UI design from the Knowledge Base is retained as the functional specification and re-expressed as QML dockable panels (UI_GUIDELINES.md).
**Why Qt wins:** (a) dense 10–50 Hz telemetry rendering (EFIS tapes, oscilloscopes, node graphs) is native-canvas work where Electron's compositor and GC pauses fight you; (b) offline/air-gap by construction, no bundled Chromium attack surface to justify to a defense auditor; (c) one C++ toolchain across services and UI; (d) aerospace operators expect a native control-room application, not a web app in a frame.
**Cost accepted:** QML talent is scarcer than React; the interactive HTML prototypes of the 24 tabs must be re-expressed in QML (they remain valid as *functional* specs).

## ADR-0003 — Gazebo Harmonic (LTS), not Gazebo Garden
**Status: ACCEPTED (deviation from Knowledge Base, version-level only)**
**Context:** The Knowledge Base pins Gazebo Garden. Garden reached end-of-life in late 2024; Harmonic is the current LTS with PX4 SITL support.
**Decision:** Target Gazebo **Harmonic**. This is a version bump, not a technology change; pinning an EOL simulator into a greenfield platform would be an unforced error.
**Consequences:** Verify `gz-transport`/PX4 bridge versions during the Phase 3 simulation spike. ROS 2 Humble ↔ Harmonic pairing needs the `ros_gz` compatibility matrix checked (tracked with R-10).

## ADR-0004 — API gateway remains Node.js gRPC-Gateway, with reservations
**Status: ACCEPTED (revisit at Phase 4 security hardening)**
**Context:** Constitution specifies a Node.js gRPC-Gateway translating REST/WebSocket to internal gRPC. It is the only Node component in an otherwise C++/Python stack.
**Decision:** Keep it. The gateway is stateless translation + JWT validation — not latency-critical, and gRPC-Gateway tooling is mature.
**Alternatives:** Envoy with gRPC-JSON transcoding (stronger, but heavier ops burden for Phase 1); Drogon-native REST (spreads HTTP concerns into every service — violates boundary discipline).
**Reservation recorded:** a third runtime enlarges the supply-chain and FIPS story for Phase 4; if hardening makes Node costly, migrate to Envoy — the internal gRPC contracts make the gateway replaceable by design.

## ADR-0005 — NATS JetStream is the internal message bus
**Status: ACCEPTED**
**Context:** Services need pub/sub with persistence on both a Jetson-class edge node and in cloud. The original ideation chat floated ROS 2 DDS, gRPC, and ZeroMQ for internal transport.
**Decision:** NATS JetStream for all inter-service eventing; gRPC strictly for command/query.
**Alternatives rejected:** **Kafka** (JVM + broker ops on an edge box is disqualifying); **ZeroMQ** (no persistence, no subjects — we'd rebuild JetStream badly); **DDS as the internal bus** (couples every service to ROS 2 QoS semantics and discovery storms; DDS stays at the ros2-bridge boundary); **Redis Streams** (acceptable fallback, weaker consumer semantics and no subject hierarchy).
**Consequences:** One ~15 MB binary to operate; at-least-once delivery means consumers must be idempotent (EVENT_FLOW.md defines dedup keys).

## ADR-0006 — Core services in C++17 on Drogon
**Status: ACCEPTED**
**Context:** Telemetry and MAVLink paths need deterministic latency at 1000 Hz ingest on ARM64; flight-influencing code must be MISRA-checkable.
**Decision:** C++17, Drogon framework, for all Phase 1 engines. Python is confined to ai-engine and simulation orchestration where the ML/tooling ecosystem dominates and latency doesn't.
**Alternatives rejected:** **Rust** (genuinely attractive for safety; rejected because MISRA/DO-178C tooling, the Qt toolchain, and the founder's expertise are C++ — a two-language safety story doubles the certification argument); **Go** (GC pauses on the hot path; weak MISRA analog).
**Consequences:** Memory-safety burden carried by MISRA subset + static analysis + `Result<T,E>` no-exception discipline (CODING_STANDARDS.md).

## ADR-0007 — Local-first storage behind a `StorageProvider` abstraction
**Status: ACCEPTED**
**Context:** No budget for cloud during development; defense/industrial buyers require on-prem or air-gap anyway.
**Decision:** Phases 1–3 run PostgreSQL 15 + TimescaleDB + Redis 7 + MinIO entirely locally in containers. All object/blob access goes through `StorageProvider`; all relational access through service-owned repositories. No Firebase/Supabase/AWS SDK may appear in Phases 1–3.
**Consequences:** Phase 4 cloud is a backend swap plus a sync service (CLOUD_ARCHITECTURE.md), not a rewrite. "No cloud" is marketed as a feature (air-gap) rather than apologised for.

## ADR-0008 — Maps: MapLibre family + offline tile packages; no custom renderer
**Status: ACCEPTED (format detail OPEN — OQ-3)**
**Context:** A custom Canvas radar was prototyped and rejected: a dead end versus real GIS. Mapbox requires API keys (fails air-gap); Google Maps is disqualified outright.
**Decision:** MapLibre (BSD) with fully offline tile packages generated from OpenStreetMap (Planetiler/tilemaker). `MapConfig` interface defined day one with `offlineMode: true` meaning **zero network requests, non-negotiable**.
**Open detail (OQ-3):** the Qt GCS uses MapLibre **Native**, whose PMTiles story differs from MapLibre GL JS. Phase 1 spike decides PMTiles vs MBTiles vs local tile server. The `MapConfig.source` enum already covers all three — no contract change either way.

## ADR-0009 — Full telemetry Protobuf contract frozen in Phase 1
**Status: ACCEPTED**
**Context:** Retrofitting fields (RTK, jamming/spoofing, per-cell battery, per-motor ESC) into a live telemetry pipeline breaks every consumer.
**Decision:** `VehicleTelemetry` / `TelemetrySnapshot` / `TelemetryDelta` are fully specified in Phase 1 (census in TELEMETRY_ENGINE.md), versioned, additive-only within v1.
**Consequences:** Slightly heavier Phase 1 schema work; permanent stability for every downstream consumer including FlightMD integration and the digital twin.

## ADR-0010 — DO-178C-aligned process, with the DO-278A mapping recorded
**Status: ACCEPTED**
**Context:** The constitution mandates DO-178C alignment. Formally, DO-178C governs *airborne* software; UAOP is ground software, whose analog is DO-278A/ED-109A. An auditor will notice.
**Decision:** Apply DO-178C process objectives as mandated (plans, traceability, verification, CM, QA), and maintain an explicit DO-178C→DO-278A objective mapping in COMPLIANCE.md so the artifacts serve either framework. DAL C-equivalent rigor for flight-influencing modules; DAL D-equivalent elsewhere. DO-330 applies to the RTM generator if its output enters certification data. ARP4754A/ARP4761 concepts govern platform-level requirements and the FHA.
**Consequences:** Honest posture ("certification-ready, not certified"); no wasted work if a customer's basis of certification names either standard.

## ADR-0011 — Orchestration ladder: Compose → k3s → Kubernetes
**Status: ACCEPTED**
**Decision:** Workstation = Docker Compose (one command, zero cluster ops). Edge Node = **k3s** (single-binary Kubernetes: OTA-friendly rollouts, restart policies, node drain — without full k8s weight on a Jetson). Cloud = managed Kubernetes.
**Alternatives rejected:** systemd units on edge (no declarative rollback); full kubeadm on edge (ops burden); Nomad (smaller ecosystem, second orchestrator to document).
**Consequences:** One manifest lineage (Kustomize base + overlays) spans edge and cloud.

## ADR-0012 — AI is advisory-only, structurally
**Status: ACCEPTED (safety + certification strategy)**
**Decision:** No AI output can reach a vehicle without an explicit operator action. Enforced structurally: ai-engine has **no NATS publish permission on `uaop.cmd.>`** subjects; recommendations flow only via `uaop.ai.>` to the UI. Confidence score + model version on every artifact; SHOWN/APPLIED/DISMISSED audit-logged.
**Consequences:** Keeps ML models out of the flight-influencing assurance boundary — the certification argument covers the *gate*, not the model.

## ADR-0013 — MISRA C++ subset; no exceptions or dynamic allocation on real-time paths
**Status: ACCEPTED**
**Decision:** Flight-influencing C++ uses a MISRA C++ ruleset enforced in CI from commit one; real-time paths use pre-allocated pools and `Result<T,E>` instead of exceptions. Retrofitting either is effectively impossible — hence day one.

## ADR-0014 — ai-engine consumes `flightmd_core` as a package
**Status: ACCEPTED**
**Decision:** Log analysis (FFT oscillation, vibration, EKF, battery, GPS, parameter anomaly, motor/ESC — 7 modules) comes from the FlightMD project's pure-Python `flightmd_core` package, imported directly (Mode A: offline, air-gap-safe). `FlightMDReport` schema v1.0 is a frozen cross-project contract. `flightmd_core` must never import web frameworks.
**Consequences:** FlightMD's real-world users continuously harden UAOP's AI layer before UAOP ships it.

---

## OPEN items (decisions required — do not assume)

| ID | Question | Owner | Needed by |
|---|---|---|---|
| OQ-1 | `dji-service/`, `fpv-service/` exist in the repo skeleton but in no planning document. In scope? DJI MSDK vs Cloud API? Analog FPV or digital (DJI O3/HDZero)? Recommend: park both as empty adapters, decide at Phase 2 planning. | Founder | Phase 2 planning |
| ~~OQ-2~~ | **RESOLVED 2026-07-04 (R2/F16):** `middleware/mavlink-bridge/` created during the workspace-structure pass, per the founder's directive to establish the full project structure. MAVLink parsing stays out of telemetry-engine; the adapter boundary holds. | — | done |
| OQ-3 | Offline tile format under MapLibre Native/Qt (PMTiles vs MBTiles vs local server). Needs a 2-day spike. | Eng | Phase 1 map panel |
| OQ-4 | Platform licensing: open-core (core Apache-2.0, enterprise closed) vs fully proprietary. Shapes SDK, marketplace, and community strategy. | Founder | Phase 4 planning |
| OQ-5 | BUSINESS_MODEL.md pricing figures are architect proposals, not founder-validated. | Founder | Before first sales conversation |
| OQ-6 | **GCS emergency direct-connect** (from R1 finding F9): should the GCS embed a minimal MAVLink client to preserve situational awareness + EMERGENCY commands if the edge node dies mid-flight? Real safety value for single-node deployments vs a second MAVLink implementation inside the assurance boundary. | Founder | Phase 2 planning |
| OQ-7 | **UI plugin isolation** (from R1 finding F10): declarative-only panel schema vs out-of-process rendering vs signed-trust acceptance. Current in-process QML model is the weakest sandbox wall. | Founder + eng spike | Phase 4 planning |

---

# Critical Review Log

Working method: **every document generation is followed by a critical architecture review** (weaknesses, hidden assumptions, missing requirements, scalability, security, certification, technical debt). Findings land here with dispositions; accepted design changes become ADRs; the affected documents are edited in the same pass. Process definition: [DOCUMENTATION_PROCESS.md](../08_Developer_Handbook/DOCUMENTATION_PROCESS.md). The first design is never assumed to be the best design.

## Review R1 — full v0.1.0 baseline · 2026-07-03

| # | Lens | Finding | Disposition |
|---|---|---|---|
| F1 | Weakness / safety | Fail-closed audit coupled **command availability to PostgreSQL health** — a DB outage would have blocked an emergency RTL. Safety regression hiding inside a compliance virtue. | **ADR-0015**; MICROSERVICES, COMPLIANCE, EDGE_ARCHITECTURE corrected |
| F2 | Hidden assumption | One operator/GCS per vehicle was assumed everywhere; no ownership or handover model for multi-seat control rooms — yet control rooms are the stated design inspiration. | **ADR-0017**; UAOP-HLR-005 added |
| F3 | Missing requirements | No NFR for GCS rendering performance; no requirement for platform time discipline (was only prose in HARDWARE_ARCHITECTURE §5). | UAOP-NFR-011/012 added to PRODUCT_REQUIREMENTS |
| F4 | Scalability / honesty | AUDIT stream described as "replicated PVC" — replication is meaningless on a single-node edge; the durability story there is fsync + backup + chain verification. | EVENT_FLOW.md wording corrected |
| F5 | Security | Phase 1 local JWT had no revocation story — a stolen token lived until expiry. | SECURITY.md: short-lived access tokens (≤60 min) + refresh + Redis denylist; full fix arrives with OIDC (Phase 4). Interim risk accepted and recorded |
| F6 | Certification | TESTING.md claimed "MC/DC-oriented" coverage for DAL C-equivalent code. MC/DC is a DAL A/B objective — overclaiming invites exactly the auditor skepticism we build to avoid. | TESTING.md corrected to decision coverage |
| F7 | Certification / standards | The Knowledge-Base-era MISRA C++ **2008** predates C++11 and fights the C++17 idiom canon (RAII, variant/optional). | **ADR-0016**: MISRA C++:2023 |
| F8 | Tech debt | compliance-engine as a mixed C++/Python service blurs its DAL C-equivalent boundary — the audit chain and the SORA PDF generator do not belong in one assurance scope. | Split recorded: chain/audit core = C++ (DAL C-equiv); SORA calculators/report generation = Python sidecar (DAL D-equiv). MICROSERVICES entry to be updated when the service is scaffolded |
| F9 | Weakness / availability | Edge-node death mid-flight leaves the operator blind — GCS is a thin client; autopilot failsafes protect the aircraft but not situational awareness. | **OQ-6** opened; founder decision |
| F10 | Security | In-process QML plugins are the weakest wall in PLUGIN_SYSTEM.md §3 — "capability-restricted API" is not process isolation. | **OQ-7** opened with options |
| F11 | Hidden assumption | vehicle-manager restart (≤5 s) creates a command-authority gap; no HA pair exists on a single edge node. | **Accepted**: gap is bounded, alarmed, and autopilot failsafes are independent by design. Recorded so nobody rediscovers it as a surprise |
| F12 | Tech debt / scope | Importing the full 24-panel functional census keeps R-1 (scope gravity) hot for Phase 1. | Enforced subset in [IMPLEMENTATION_PLAN.md](../07_Implementation_Guides/IMPLEMENTATION_PLAN.md); census retained as roadmap, not obligation |

---

## ADR-0015 — Fail-closed boundary moves from Postgres append to durable event persistence
**Status: ACCEPTED (R1/F1) — supersedes the fail-closed wording in v0.1.0 of MICROSERVICES, COMPLIANCE, EDGE_ARCHITECTURE**
**Context:** The original rule — refuse commands when the audit chain cannot be written — bound command availability to PostgreSQL. Review found the safety inversion: an operator unable to command RTL because a database is restarting.
**Decision:** A vehicle-bound command is admissible when its audit event is **durably persisted (fsync) in the JetStream AUDIT stream**. The Postgres chain append is asynchronous; append lag is a monitored golden signal with CAUTION at threshold. An **EMERGENCY command class** (RTL, LAND, revert-to-manual) additionally survives full bus failure via a bridge-local append-to-disk WAL, reconciled into the chain on recovery.
**Consequences:** The auditability guarantee is preserved — the event is durably recorded before the action, and the chain is provably complete after reconciliation. PostgreSQL leaves the flight-influencing availability path. The WAL + reconciliation code enters DAL C-equivalent scope (a fair trade: small, testable, and it buys emergency-command availability).

## ADR-0016 — Standards modernization: MISRA C++:2023; C++17 services, C++20 GCS on Qt 6.8 LTS
**Status: ACCEPTED (R1/F7 + state-of-the-art mandate with Qt/QML + C++ locked)**
**Decision:** Flight-influencing services remain **C++17**, checked against **MISRA C++:2023** (the current standard, written for modern C++; the 2008 edition predates C++11 and would outlaw the codebase's own idiom canon). The GCS frontend moves to **C++20 on Qt 6.8 LTS** — it is DAL D-equivalent, not MISRA-bound, and benefits concretely (ranges for model pipelines, designated initializers for config structs, coroutines for async gateway clients). Single toolchain: GCC 12+/Clang 16+ covers both.
**Alternatives rejected:** uniform C++20 including services (static-analysis and MISRA-checker maturity for C++20 constructs in a safety context is still uneven — revisit at Phase 3); staying silent on the MISRA edition (an auditor's first question).

## ADR-0017 — ControlSession: explicit command authority and handover
**Status: ACCEPTED (R1/F2)**
**Context:** Multi-seat operation is a target deployment (control-room inspiration; the multi-GCS conflict scenario was already acknowledged in vehicle-manager's failure modes) but ownership was never modeled — "serialize and log both" is not an authority model.
**Decision:** vehicle-manager owns a **ControlSession** per vehicle: exactly one CONTROLLING session; all others OBSERVING. Non-controlling commands are refused with `NOT_IN_CONTROL` — except the EMERGENCY class, which any authenticated PILOT-role session may issue (with immediate alert to the controlling seat, both audited). Handover: request→accept; supervisor-forced (FLEET_MANAGER/ORG_ADMIN) with mandatory reason; or automatic on controlling-session death → vehicle enters **UNCONTROLLED** with a WARNING-level alert rather than silently transferring. All transitions audited.
**Consequences:** Gateway sessions carry identity beyond authentication (session id in every command envelope); the GCS gains a persistent controlling/observing indicator (UX addition, Phase 1 scope); requirement UAOP-HLR-005 traces this.

## Review R2 — workspace inventory & structure pass · 2026-07-04

Trigger: founder directive to inventory the main checkout, keep the usable, delete the unnecessary, and realize the full project structure.

| # | Lens | Finding | Disposition |
|---|---|---|---|
| F13 | Discovery / tech debt | An untracked **Qt GCS prototype** existed in the main checkout (CMake + C++ TelemetryController + QML views: Telemetry/Mission/Compliance/Defence). Quality is genuinely good — Qt 6, C++20, `qt_add_qml_module`, warnings-as-errors — coincidentally matching ADR-0016 before it was written. | **KEPT** as the M0 GCS seed. Debt recorded: view-per-page layout predates the dockable-panel architecture (UI_GUIDELINES §2) and QML views bind no C++ models yet — refactor lands in M0/M2. The "Defence" view predates phase scoping (defense is Phase 4+ market, not a Phase 1 panel) — parked, not deleted. |
| F14 | Hidden assumption / tech debt | **Five parallel AI-tool context files** (CLAUDE.md, AGENTS.md, GEMINI.md, .cursorrules, .windsurfrules) carried stale, mutually contradictory instructions — Next.js dashboards, qmake builds, git submodules of ArduPilot/QGC forks (violating ADR-0001), a `frontend/ui-nextjs` that never existed. Multiple sources of truth is exactly the failure MASTER_CONTEXT exists to prevent. | Archived to `docs/context/legacy/`; fresh CLAUDE.md + AGENTS.md regenerated from the doc set (read order, hard rules, frozen dirs, current milestone). Cursor/Windsurf/Gemini configs can be regenerated from CLAUDE.md if those tools return. |
| F15 | Repo hygiene / licensing | `external-resources/` held ~7 **non-UAV repositories** (agent-skills, SEO tooling, Excel MCP, design references); the initial commit had tracked 302 of their skeleton `.gitkeep` files — noise in every `git status`, plus third-party licensing exposure in our tree. | Content removed everywhere; directory repurposed with a tracked README stating the rules (UAV reference material only, git-ignored, never built); `.gitignore` enforces. |
| F16 | Decision closure | OQ-2 (mavlink-bridge directory) blocked on a founder confirm; the structure directive supplies it. | **OQ-2 RESOLVED** — directory created; MASTER_CONTEXT/MICROSERVICES/PROJECT_STRUCTURE markers cleared. |
| F17 | Certification / architecture | The prototype `docker-compose.yml` ran **rosbridge websocket as the core transport** — a pattern that would bypass the audited gateway command path entirely (no authority checks, no audit events on rosbridge traffic). | Archived with the legacy set; DEPLOYMENT.md's compose profile (NATS/gRPC architecture) supersedes it. Recorded so nobody resurrects it as a "quick demo transport". |
| F18 | Security / hygiene | **No `.gitignore` existed**: `.claude/` worktree state, `.code-review-graph/`, and 18.9 MB of `frontend/.../build/` artifacts were one careless `git add -A` away from the history. | Root `.gitignore` added (tool state, build outputs, generated proto code, external-resources content, OS noise); build artifacts deleted. |
| F19 | Missing structure | 30+ directories specified in PROJECT_STRUCTURE.md did not exist (proto packages, five services, mavlink-bridge, integrations, k8s overlays, monitoring, test tiers, compliance tooling, docs/compliance). | Full tree created with `.gitkeep`s; FROZEN.md guards placed in dji-service/fpv-service per PROJECT_STRUCTURE §2. |
