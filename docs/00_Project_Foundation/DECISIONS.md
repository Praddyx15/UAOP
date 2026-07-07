# DECISIONS — Architecture Decision Records

**Version 0.1.5 · 2026-07-07 · 20 ADRs, 6 reviews (R1–R6) to date.**

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
| ~~OQ-3~~ | **RESOLVED 2026-07-07 (R6/ADR-0020):** MapLibre Native Qt's prebuilt binaries top out at Qt 6.7 vs our Qt 6.11.1 kit — no ABI-safe prebuilt. Interim `QQuickImageProvider` + MBTiles/SQLite raster renderer shipped instead; `MapConfig.source: 'mbtiles_local'` implemented. Full MapLibre integration re-spiked when a matching prebuilt exists or as its own timeboxed source-build attempt. | — | done |
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

## Review R3 — IMPLEMENTATION_PLAN.md v0.2.0 expansion · 2026-07-06

Trigger: founder request for a detailed, thorough implementation/development plan. The v0.1.0 plan had milestone-level objectives only (no task IDs, no effort estimates, no timeline); expanding it to a task-level WBS surfaced process gaps the milestone-level view had hidden.

| # | Lens | Finding | Disposition |
|---|---|---|---|
| F20 | Certification / sequencing | The RTM generator (`compliance/tools/rtm-generator`) was scheduled at **M7**, but `@req` annotations start accumulating at **M2** (first flight-influencing code). Traceability retrofitted at the end is exactly what UAOP-NFR-009 and COMPLIANCE.md §A.3 forbid — "never retrofitted" was true in prose but not in the schedule. | Moved to **M0.8**: scaffold + CI lint land before M2 writes any annotated code. M7.7 becomes a verification step only. |
| F21 | Security / process gap | Fuzz corpus is seeded at M2.3, but the plan had no CI job running it until "fuzz campaign review" at **M7.4** — a 4–5 month gap where SECURITY.md §7's "fuzzed in CI" claim would have been false in practice. | Nightly fuzz-job stub added to **M0.5** (empty corpus initially); M2.3 seeds it; M7.4 becomes a review of an already-continuously-run campaign, not its first run. |
| F22 | Weakness / estimation honesty | Effort figures are architect judgment with no empirical basis — presenting a single "4–5 months" figure (as v0.1.0 did) implies false precision the WBS cannot support. | Replaced with three explicit cadence scenarios (5.3 / 8.8 / 13.3 months) and a mandatory **recalibration checkpoint (M2.10)** after the first vertical slice, before M3–M7 estimates are trusted further. |
| F23 | Weakness / continuity | The critical path (M1→M2→M3→M4→M5) is fully serial; combined with [RISK_REGISTER R-4](RISK_REGISTER.md) (solo-founder bandwidth competing with job search), an interruption to core-path work stalls the *entire* plan with nothing else to absorb the time. | §2 of IMPLEMENTATION_PLAN.md now names explicit zero-dependency "bench tasks" (metadata packs, SITL scenario seeding, docs, GCS fixture-driven polish) to keep momentum during forced core-path gaps. |
| F24 | Security / enforcement gap | PROJECT_STRUCTURE.md §2 asserts "CI fails on any other file" in the frozen `dji-service/`/`fpv-service/` directories, but no task anywhere built that CI check — the guard was a stated intention, not a mechanism. | Added to **M0.5** (build the guard) and **M5.7** (re-verify it before Phase 2 planning touches OQ-1 scope). |
| F25 | Scalability / process | Full SITL validation-matrix automation was a single 4-day task at **M7.1** — all integration-test infrastructure built in one push, immediately before the gate it's meant to prove, with no earlier signal if an engine's test surface was hard to automate. | Distributed: each engine milestone (M2.9, M3.7, M4.8, M5.8) adds its own slice to `tests/sitl/` as it lands; M7.1 drops from ~4d to 2d ("assemble and verify" instead of "build from scratch"), and problems surface incrementally instead of all at once at the gate. |

**Net effect on the plan:** Phase 1 total effort is ~115 focus-days with these gaps closed in the schedule rather than left as prose commitments — the same failure mode (documentation claims that don't have a corresponding scheduled task) had already been called out qualitatively in the constitution ("a doc that disagrees with code is a P1 defect"); this review extends the check to "a doc that disagrees with the *schedule* is the same defect, just not caught yet."

## ADR-0018 — flightmd_core is real and live; integration parameters corrected, AI-disabled-by-default confirmed
**Status: ACCEPTED**
**Context:** ADR-0014 committed to consuming `flightmd_core` as a package based on the *plan* for FlightMD as it existed when UAOP's documentation baseline was written. Inspection of the actual project (`C:\Users\akass\Downloads\flightmd`, 2026-07-07) shows FlightMD is now a real, live, independently shipped product at [github.com/Praddyx15/FlightMD](https://github.com/Praddyx15/FlightMD) (MIT), deployed at flightmd.vercel.app / flightmd-api.onrender.com, materially more capable and different in specifics than assumed: schema `v1.5` (not v1.0), three log formats auto-detected — PX4 ULog, ArduPilot DataFlash, **and MAVLink telemetry `.tlog`** (not two), an eighth unweighted analyzer (ascent/recovery, for rockets/HABs), and an AI-enhancement layer that defaults to **Groq** (free tier) with Anthropic Claude optional — not Claude-mandatory as UAOP's docs previously implied. It has also been validated against 50 real-world PX4 logs spanning 11 vehicle types, sourced from PX4's own public Flight Review database.
**Decision:**
1. AI_ENGINE.md, MASTER_CONTEXT.md §10, PRODUCT_REQUIREMENTS.md UAOP-HLR-031, and RISK_REGISTER.md R-6 are corrected to the real package (done in this pass).
2. **UAOP's Mode A integration passes no `AIEnhancer` by default** — the entire post-flight analysis path (deterministic rules + plain-English text, which `flightmd_core` generates without any AI call) runs at zero network calls in every profile, satisfying UAOP-NFR-001 and SECURITY.md's flight-data-confidentiality asset without special-casing. Enabling an Anthropic-backed `AIEnhancer` for polished prose is an explicit, per-organisation opt-in when the node has connectivity — never a platform default.
3. **Scope boundary:** UAOP imports only the `flightmd_core` Python package. FlightMD's own web API, frontend, and fleet-operations features (per-airframe maintenance tracking, webhook alerts, cross-flight trends, dataset contribution) are **not** consumed by UAOP and are out of scope for it — those exist to serve FlightMD's own standalone users. UAOP's FLEET_MANAGEMENT.md and COMPLIANCE.md remain sole authority for fleet/compliance concerns inside UAOP.
4. **Reusable non-code assets imported now, ahead of their scheduled milestones** (small, MIT-licensed, same author, no third-party licensing question): `tests/data/sample_logs/` (6 clean/flawed fixture logs across all three formats + the generator script, ~1.5 MB) and `backend/services/parameter-engine/data/` (PX4 v1.13/v1.14 default-parameter and safe-range JSON, ~7 KB) — both with provenance READMEs. The `flightmd_core` Python source itself, its API, and its frontend are **not** vendored into `uaop-platform` (ADR-0001 adapter discipline; the existing monorepo exception in REPOSITORY_STRUCTURE.md §2 rule 1 already commits to consuming it as a pinned dependency, not a copy).
5. The much larger real-world validation corpus (50 flights, ~8.5 GB) at the source project is **not** imported — no Git LFS is configured yet, and importing multiple gigabytes of binary data is a hard-to-reverse repo-bloat action nobody asked for. It is recorded as a future bench-task candidate (IMPLEMENTATION_PLAN.md §2), not scheduled.
**Consequences:** Phase 2's ai-engine build (M9, not yet detailed) starts from a corrected, de-risked specification instead of a stale plan; the air-gap posture is strengthened, not weakened, by the real dependency's own AI-optional design. See Review R4 below for the full seven-lens pass this inspection triggered.

## Review R4 — FlightMD real-project inspection · 2026-07-07

Trigger: founder question "can we use `C:\Users\akass\Downloads\flightmd` anywhere in our project?" Answer: yes, and the inspection required to answer it honestly surfaced that UAOP's documentation had been describing FlightMD's *plan*, not its shipped reality, since Phase 0.

| # | Lens | Finding | Disposition |
|---|---|---|---|
| F26 | Weakness / scope mismatch | FlightMD's real 8th analyzer (ascent/recovery, for rockets and HABs) runs on every flight by default; UAOP has no rocket/HAB vehicle profile and would surface irrelevant findings to operators. | ai-engine's integration wrapper filters `Category.ASCENT_PROFILE` findings out of the UAOP-facing report — a filter at the integration boundary (adapter pattern), not a change to `flightmd_core` itself. Documented in AI_ENGINE.md §2.1. |
| F27 | Hidden assumption (corrected, favourably) | AI_ENGINE.md v0.1.0 implied Claude API calls were necessary for post-flight report explanations. Reality: `flightmd_core` generates plain-English text via its deterministic rule engine with **no AI call at all** — AI is a pure prose-polish option, defaulting to Groq upstream. Had this gone uncorrected, a Phase 2 implementer following the old doc could have wired in a hard Anthropic dependency, silently violating ADR-0007/UAOP-NFR-001 (air-gap). | AI_ENGINE.md §2.1/§6 corrected; ADR-0018 makes "no `AIEnhancer` by default" an explicit rule, not an inference. |
| F28 | Missing requirement | No UAOP requirement stated that post-flight analysis must work with zero network calls by default — HARDWARE/AI_ENGINE prose implied it but PRODUCT_REQUIREMENTS didn't say so. | UAOP-HLR-031 amended with an explicit offline-by-default clause. |
| F29 | Security | `flightmd_core`'s optional AI path sends flight-summary text to a third-party API (Groq or Anthropic) when enabled. SECURITY.md ranks flight-data confidentiality as asset priority #3; enabling AI-enhancement by default in any profile — including "cloud-connected" edge nodes — would leak operational summaries off-node without an explicit decision ever having been made. | Made explicit and structural in ADR-0018: off by default everywhere, per-org opt-in only, never inferred from connectivity state alone. |
| F30 | Certification | `flightmd_core` is plain Python with no MISRA/DO-178C-equivalent process behind it — importing it into a DAL-C-equivalent path would be a real problem. It sits entirely inside ai-engine, already DAL D-equivalent (advisory-only, COMPLIANCE.md §A.2) and structurally walled off from `uaop.cmd.>` (ADR-0012). | No architecture change needed — confirmed the existing wall is exactly what makes this safe, and said so explicitly rather than leaving it implicit. |
| F31 | Tech debt / future confusion | FlightMD has independently grown fleet-ops features (maintenance tracking, webhooks, trends, PDF/GPX export) that conceptually overlap with UAOP's own FLEET_MANAGEMENT.md/COMPLIANCE.md. Without a stated boundary, a future contributor could be tempted to consume FlightMD's API for "fleet stuff too," creating two divergent, unsynchronized fleet-management implementations. | Explicit boundary rule recorded in ADR-0018 and AI_ENGINE.md §6: UAOP imports only the `flightmd_core` package, never FlightMD's API/frontend/fleet features. |
| F32 | Scalability (positive finding) | Real-world validation evidence (50 logs, 11 vehicle types, up to ~270 MB/long-duration files per the source project's own verification report) is stronger evidence for `flightmd_core`'s robustness than UAOP's own docs previously had reason to claim. | RISK_REGISTER R-6 narrowed to scope the residual risk to the UAOP-native LSTM tier only — the deterministic layer is no longer a source of uncertainty in that risk's scoring. |

**Net effect:** no architecture changed as a result of this review — every finding was a *correction toward reality* (favourable in most cases: stronger air-gap story, validated dependency, clearer boundaries) rather than a new design flaw. That is itself worth recording: not every review finds a defect to fix; some findings confirm that inspecting reality was overdue.

## ADR-0019 — License-tiered reuse of ecosystem source code (PX4/QGC portable; ArduPilot/Mission Planner never in-tree)
**Status: ACCEPTED · 2026-07-07**
**Context:** Founder proposal to accelerate development by taking code from ArduPilot, Mission Planner, QGroundControl, PX4-Autopilot, and PX4-ECL — "not copy them, but take the necessary code, refine it for our need." The legal reality that decides this: *taking and refining* source code is creating a **derivative work** under copyright law, regardless of how much it is refined — the upstream license governs the result. The five repos split cleanly (licenses verified against the live repos, 2026-07-07):

| Repo | License (verified) | Verdict |
|---|---|---|
| PX4-Autopilot | BSD-3-Clause | **Tier 1** — extract, refine, use freely; attribution preserved |
| PX4-ECL | BSD-3-Clause · **archived May 2024**; EKF2 now lives in PX4-Autopilot `src/modules/ekf2` | Tier 1, but source anything "from ECL" out of PX4-Autopilot mainline instead — the ECL repo is dead (R5/F36) |
| QGroundControl | **Dual Apache-2.0 / GPL-3.0** | **Tier 2** — usable under the Apache option with per-file license verification (some files/third-party components may be GPL-only) |
| ArduPilot | GPL-3.0 | **Tier 3 — never in-tree**, in any form |
| Mission Planner | GPL-3.0 (and C#, wrong language regardless) | **Tier 3 — never in-tree** |

**Decision — the three tiers:**
1. **Tier 1 (permissive: BSD/MIT/Apache-only):** code and data may be extracted, ported, and adapted. Every extraction carries: a provenance header in the file (source repo, commit, original license), an entry in a root-level `THIRD_PARTY_NOTICES.md`, and the upstream license text preserved. Porting into DAL C-equivalent paths additionally requires full MISRA/`@req`-annotation treatment at port time — **no "it's from PX4 so it's fine" exemption**; imported code passes the same gates as first-party code (R5/F38).
2. **Tier 2 (dual/mixed licensing):** as Tier 1, plus mandatory per-file license verification before extraction (QGC's Apache option does not automatically cover every file or bundled third-party component), recorded in the notices entry.
3. **Tier 3 (GPL/AGPL copyleft):** **never enters the uaop-platform tree — including "refined," restructured, or AI-transformed versions.** A GPL derivative would relicense UAOP's distribution as GPLv3, which (a) silently decides OQ-4 (open-core vs proprietary — an open founder decision) as a side effect of a code-borrowing convenience, (b) forecloses the Phase 4 closed enterprise/defense components entirely, and (c) is effectively irreversible once shipped. Tier 3 projects remain the canonical **behavioral references**: mode semantics, protocol quirks, tuning workflows are *facts*, and facts are not copyrightable — their *expression in code* is. Study lives in `external-resources/` (git-ignored); transcription is prohibited.
4. **Prompt-hygiene rule for AI-assisted work (R5/F34):** never paste Tier 3 source into any AI or code-generation tool — asking a tool to "refine this GPL function for our codebase" produces a derivative work with extra steps. Describe the observed behavior in words or via the protocol specification, then implement fresh. This rule is in CLAUDE.md/AGENTS.md as a hard rule because AI-assisted workflows make this the single most likely accidental-contamination path.
5. **Data vs code:** parameter names/ranges/defaults, MAVLink message definitions, and airframe metadata are facts published by upstreams in machine-readable form *for exactly this consumption* (every GCS uses them). PX4 parameter metadata, MAVLink XML (already pinned per MAVLINK_INTEGRATION.md §1), and ArduPilot's published parameter-definition data are consumed as data regardless of host-repo code license — recorded here as accepted, industry-standard practice.
6. **Enforcement:** the M0.5 CI shell gains an **in-PR license-allowlist gate** (permissive-only headers for third-party-derived files; GPL-text denylist) — the existing release-time license audit alone would catch contamination months late (R5/F35).

**High-value extraction targets (making the acceleration concrete, not hypothetical):**
- **PX4-Autopilot (Tier 1):** parameter metadata + airframe configs → parameter-engine metadata packs (M4.4, extends the FlightMD-seeded data); failsafe/mode/events enums → vehicle-manager decode tables (M2.5/M3.3); EKF2 fault-flag semantics (ex-ECL) → telemetry `ekf` category decode; SITL consumed as a container (already planned, unaffected).
- **QGroundControl (Tier 2):** its `MissionManager`/`ParameterManager` transfer-and-retry state machines are the ecosystem's battle-tested reference for the exact protocol code UAOP must get right (M4.2 verified mission transfer, M4.4 parameter I/O) — **ported as logic into our `Result<T,E>`/no-exception idiom, never dropped in wholesale** (QGC code is exception-throwing and Qt-object-entangled; MISRA paths couldn't absorb it raw even if we wanted to). This is the single biggest schedule win available: protocol state machines are the highest-risk hand-rolled code in Phase 1.
- **ArduPilot / Mission Planner (Tier 3):** behavioral reference only — per-stack quirk tables (MAVLINK_INTEGRATION.md §2), arming-check semantics, tuning workflow benchmarks (TUNING_ENGINE.md §6). Read, understand, document the fact, implement fresh.

**Consequences:** Development genuinely accelerates where licenses permit — with the honest caveat that on MISRA paths even Tier 1 code is *adapted*, not pasted, so the win is "port a proven design" (worth weeks on the transfer protocols) rather than "skip the work." The GPL wall keeps OQ-4 a real decision instead of an accident. ADR-0001 (adapters, never forks) is unchanged: this ADR governs *file-level porting with provenance*, not dependency-taking or forking — three distinct things, now each with a written rule.

## Review R5 — upstream code-reuse proposal · 2026-07-07

Trigger: founder proposal to lift-and-refine code from five ecosystem repos for development speed. The proposal is half right — and the half that's wrong would have been catastrophic, which is exactly why it earns a review.

| # | Lens | Finding | Disposition |
|---|---|---|---|
| F33 | Hidden assumption / business | "Take and refine" treats all five repos as equivalent; two of them (ArduPilot, Mission Planner) are GPLv3, and a single refined function from either would relicense UAOP's distribution — **deciding OQ-4 (open-core vs proprietary) by accident** and foreclosing Phase 4 closed components. | **ADR-0019** Tier 3 wall; OQ-4 remains a real founder decision. |
| F34 | Weakness / tooling process | AI-assisted workflows make license contamination *easier than ever*: pasting GPL source into a generation tool and asking for a "refined" version produces a derivative work that looks original. No existing rule addressed this. | Prompt-hygiene rule in ADR-0019 §4 + hard-rule line added to CLAUDE.md/AGENTS.md. |
| F35 | Security / enforcement gap | License compliance existed only as a **release-time** audit (CI_CD.md release pipeline) — contamination would be caught months after merge, when extraction is painful. | In-PR license-allowlist gate + GPL-text denylist added to M0.5; `THIRD_PARTY_NOTICES.md` mechanism defined from first extraction. |
| F36 | Currency | PX4-ECL was **archived May 2024**; EKF2 lives in PX4-Autopilot `src/modules/ekf2`. Treating ECL as a live source would port stale estimator code. | Candidate list corrected: anything "from ECL" sources from PX4-Autopilot mainline. |
| F37 | Certification / licensing nuance | QGC's dual license is repo-level; individual files and bundled third-party components can be GPL-only. A repo-level "it's Apache" assumption would leak Tier 3 code through a Tier 2 door. | Per-file verification requirement in ADR-0019 §2, recorded per extraction in the notices file. |
| F38 | Certification | Imported Tier 1 code could be assumed "already good" and skip first-party gates — but PX4 code is not MISRA C++:2023-checked, not `@req`-annotated, and often heap/exception-idiomatic. | ADR-0019 §1: imported code entering DAL C-equivalent paths gets full MISRA + traceability treatment at port time; the port effort is the price of admission, stated in the plan rather than discovered in review. |

**Net effect:** the founder's instinct (don't reinvent battle-tested protocol code) is adopted where the ecosystem's licensing was designed for it — PX4's BSD and QGC's Apache option exist precisely so platforms like UAOP can build on them. The instinct is blocked where following it would quietly convert UAOP into a GPL project. Speed gained; company kept.

## ADR-0020 — Interim raster map renderer (QQuickImageProvider + MBTiles); MapLibre Native Qt deferred
**Status: ACCEPTED · resolves OQ-3 · 2026-07-07**
**Context:** ADR-0008 committed to MapLibre (family) with the specific Qt-side format detail left open as OQ-3. Verified against the live project (2026-07-07): MapLibre Native Qt's newest prebuilt binaries target **Qt 6.5.3/6.6.3/6.7**; the local development kit is **Qt 6.11.1**. Qt QML-facing modules are not guaranteed binary-compatible across minor versions this far apart — using the prebuilt would risk undefined behavior or load failure, and building MapLibre Native Qt from source (git submodules, multi-stage CMake, historically Linux/macOS/mobile-first validation) is a multi-hour, failure-prone undertaking that would consume the entire M0.7 timebox on a dependency that gates nothing else in Phase 1.
**Decision:** Ship an **interim raster tile renderer** for the M0/M1 development loop:
1. `MbtilesProvider` (`frontend/qt-desktop-gcs/src/map/`) — a `QQuickImageProvider` backed by `QSqlDatabase("QSQLITE")`, already bundled with Qt (no new dependency). QML requests `image://mbtiles/<z>/<x>/<y>`; the provider resolves it against a local MBTiles file, flipping XYZ→TMS row order at the DB boundary. Malformed or missing tiles degrade to a labeled placeholder — never null, never a crash (UX_GUIDELINES.md §3 absence discipline, applied to map tiles).
2. `SlippyMath.h` — pure lon/lat↔tile-fraction functions (Web Mercator), zero Qt dependency, unit-tested in isolation.
3. `MapView.qml` — a minimal pan (drag) / zoom (wheel) tile-grid view registered as the `"map"` panel through the same `PanelRegistry` every other panel uses — **the panel-host framework (M0.4) required no changes** to accommodate it, which is the architectural point.
4. **`MapConfig.source` is unaffected** (ADR-0008 already enumerated `mbtiles_local` in that interface) — swapping the renderer later is a registered-panel swap behind the same contract, not an architecture change.
**Alternatives rejected:** forcing the prebuilt MapLibre binary against a mismatched Qt minor version (silent ABI risk, deferred failure); downgrading the whole Qt kit to 6.7 to match (loses Qt 6.11 improvements platform-wide for one dependency); building MapLibre Native Qt from source now (real option, but belongs to a dedicated, timeboxed spike of its own — tracked below, not absorbed into M0).
**Consequences:** UAOP does not depend on MapLibre Native Qt building successfully before Phase 1 can proceed. The real offline-basemap story (OpenStreetMap-derived MBTiles/PMTiles packages, vector styling, GeoJSON overlays) still belongs to MapLibre and lands at **M2.7** (full map panel) — this interim renderer is deliberately raster-only and does not attempt vector cartography (the custom-Canvas-radar "dead end" ADR-0008 already rejected was specifically about vector map ambition; a bounded raster tile pyramid is a different, well-precedented scope). **Follow-up scheduled, not forgotten:** re-attempt MapLibre Native Qt either when a prebuilt binary matches the project's Qt minor version, or as its own timeboxed source-build spike — tracked as a bench-task candidate (IMPLEMENTATION_PLAN.md §2) alongside the FlightMD real-log-corpus/Git-LFS item from ADR-0018.

## Review R6 — M0.7 map spike · 2026-07-07

Trigger: OQ-3 resolution required actually attempting the MapLibre Native Qt integration rather than assuming it would work; the interim renderer that resulted is itself new code warranting the standard review.

| # | Lens | Finding | Disposition |
|---|---|---|---|
| F39 | Weakness / process | The crash investigation itself is a finding: `MbtilesProvider`'s placeholder-tile path calls `QPainter::drawText()`, which requires a font/platform backend — a test harness built on bare `QCoreApplication` (no `QGuiApplication`) crashed with an opaque OS-level exit code and zero diagnostic output, not a clean assertion failure. | Test fixed to use `QGuiApplication` with `QT_QPA_PLATFORM=offscreen` (matching the `gcs.selfcheck` pattern already established in M0.4); recorded so the next GUI-touching unit test doesn't lose an hour to the same silent crash. |
| F40 | Hidden assumption | ADR-0008's "MapLibre family" framing implicitly assumed the Qt binding would be a drop-in dependency; verifying it surfaced a real Qt-version compatibility gap that pure documentation review couldn't have caught. | General lesson, not just a map one: **ADRs that commit to a specific third-party binary/binding should be spiked against the actual local toolchain before the commitment is load-bearing**, not assumed compatible by name-recognition. No new process gate added — noted as a reviewing habit. |
| F41 | Scalability | The synthetic test tile set (85 tiles, zoom 0–3, ~104 KB) is correctly scoped as a *test fixture*, not a basemap — committed directly, no LFS needed (same reasoning as ADR-0018's FlightMD fixtures). A real operator basemap (OSM-derived, many GB at useful zoom depths) must **never** follow this path into git; it is a `MapConfig`-referenced external file per DEPLOYMENT.md, exactly like the deliberately-not-imported FlightMD validation corpus. | Stated explicitly in `tests/data/map/` generator docstring and here, so nobody later "just commits the real tiles" by analogy with the test fixture. |
| F42 | Security | `MbtilesProvider` builds SQL from a QML-supplied `id` string (`image://mbtiles/z/x/y`). Confirmed: all values are parsed to typed integers and bound via `QSqlQuery::addBindValue` (parameterized), never string-interpolated into SQL — no injection surface even though the input path originates from QML/JS. | No change needed; the existing implementation was already correct. Recorded as a verified-not-assumed security property, per DAL D-equivalent rigor (COMPLIANCE.md §A.2) applied to UI-tier code. |
| F43 | Tech debt (accepted, tracked) | The interim renderer has no tile cache eviction, no retry/backoff on DB errors, and no vehicle-position overlay — all deliberately out of scope for a spike output. | Explicitly deferred to **M2.7** (full map panel, per IMPLEMENTATION_PLAN.md) rather than half-built now; DoD for M2.7 should reference this list so the interim renderer's gaps aren't silently inherited as "done." |
| F44 | Missing structure / process gap | Two full milestones of GCS code (M0.4 panel-host, M0.7 map spike) had been verified **locally only** — `pr.yml`'s build matrix targets the headless services preset exclusively; nothing in CI installs Qt or exercises `gcs-debug`. A GCS-breaking change would merge silently until the next human happened to build it by hand. | Added a `build-gcs-linux` job to `pr.yml` (aqtinstall-provisioned Qt 6.11.1, `gcs-debug` preset, `ctest --preset gcs-debug`). Getting it green took two fix-forward iterations, both instructive: (1) `qtshadertools` is a separate addon module that Qt6's Quick CMake config depends on even for plain QML apps — confirmed via `aqt list-qt --modules`, not assumed; (2) aqt's on-disk directory name does not always match its own arch argument (`linux_gcc_64` installs to a `gcc_64` directory — read directly from aqt's source, `QtRepoProperty.get_arch_dir_name`). Fixed by **detecting** the installed directory at runtime instead of hardcoding a path, so a future aqt naming change fails loudly (a missing `lib/cmake/Qt6` check) rather than silently pointing at nothing — the same "verify against reality, don't assume compatibility" lesson as F40, applied to CI plumbing this time. |

**Net effect:** OQ-3 is resolved with working, tested code rather than a paper decision — the spike produced the artifact ADR-0008 asked for (a `MapConfig.source: 'mbtiles_local'` implementation) while correctly identifying that the originally-assumed rendering backend needs its own follow-up spike, tracked rather than silently dropped.
