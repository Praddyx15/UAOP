# ROADMAP

**Version 0.1.0 · 2026-07-03 · Four phases + Phase 0. Gates are sequential and non-negotiable (MISSION.md: depth over breadth). Durations are effort-shaped estimates for a solo founder working with AI-assisted tooling; scope, not calendar, defines a phase.**

---

## Phase 0 — Documentation baseline & skeleton *(current)*

**Objectives:** documentation set (this one) agreed; open questions dispositioned; repo skeleton completed.
**Deliverables:** all `docs/*.md`; OQ-1/OQ-2 decisions recorded; ⊕-marked directories created (REPOSITORY_STRUCTURE.md); `api/proto/` v1 package skeletons; CI pipeline shell (lint+build gates on empty-ish tree).
**Dependencies:** founder decisions on OQ-1, OQ-2.
**Risks:** documentation drift before code exists — mitigated by starting CI doc-link checks now.
**Validation:** doc review against both constitution documents; ADR audit.
**Definition of Done / Exit:** founder sign-off on the doc set; OQs resolved or explicitly deferred with dates; Phase 1 scope frozen.

---

## Phase 1 — Core GCS foundation

**Objective:** a certifiable-baseline GCS replacement: SITL-flyable, audit-chained, traceable from commit one.

**Deliverables:**
- Services: mavlink-bridge, telemetry-engine, mission-engine, parameter-engine, vehicle-manager, remote-id, flight-log-engine, compliance-engine, api-gateway — Phase 1 scope per MICROSERVICES.md.
- Full `VehicleTelemetry`/Snapshot/Delta contracts frozen (ADR-0009); NATS taxonomy live.
- Qt GCS: FLIGHT OPS + REVIEW + COMPLIANCE workspaces — panels: Flight HUD, Map (incl. OQ-3 spike resolution), Mission editor, Parameters, Sensors status, Pre-flight checklist, Remote ID, Fleet cards (edge tier), Compliance/audit viewer, Logs, Geofence, Fail-safe config, Node status.
- Workstation profile: `setup.sh` → full stack + PX4 SITL ≤ 15 min (DEPLOYMENT.md §2).
- CI: full PR pipeline incl. MISRA gate, `@req` lint, contract gate, SITL smoke.
- Compliance: PSAC, SDP, SCMP skeletons; RTM generating with ≥ 20 traced requirements.

**Dependencies:** Phase 0 exit. **Risks:** R-1 (scope gravity), R-11 (map spike), R-4 (solo bandwidth).

**Validation gate (exit criteria — all demonstrable, VALIDATION.md five layers green):**
1. PX4 SITL + ArduPilot SITL connect; 10 Hz full-state / 50 Hz attitude sustained.
2. 100-item mission upload → round-trip byte-verify → hash audit-logged.
3. Parameter write with ACK + read-back; version history queryable; failed-write path demonstrated.
4. Remote ID 1 Hz cycles evidenced in audit log.
5. Geofence breach (SITL) → FC action observed → breach event in audit chain within 1 s.
6. Link-kill test: loss detected ≤ 3 s, UI staleness correct, reconnect clean.
7. Audit chain verifier: zero gaps over the full test campaign; export bundle self-verifies.
8. Fault-injection: every service killed mid-operation, recovery ≤ 5 s, no message loss on persisted subjects.
9. MISRA zero-critical; coverage targets met; RTM ≥ 20 HLRs fully traced.
10. Disconnect test (air-gap) passes for all above.

---

## Phase 2 — Tuning depth + ROS 2 + AI diagnostics

**Objective:** outperform Mission Planner in tuning, replace RViz/rqt for ops introspection, ship advisory AI.

**Deliverables:** tuning loop end-to-end (TUNING_ENGINE.md — live response, FFT view, risk-classified writes, before/after capture, presets); ai-engine (flightmd_core integration, live oscillation, LSTM v0 with published precision/recall, PID optimiser envelope-constrained); ros2-bridge + ROS workspace (node graph, topic inspector, lifecycle, reconciliation monitor <50 ms); SORA module (SAIL calc, OSO checklist, signed PDF); BVLOS panel (dual-C2 monitor, permit tracker); TUNING + ROS workspaces; edge profile (k3s) on RC-2 reference hardware; bench HIL rig.

**Dependencies:** Phase 1 gate. **Risks:** R-6 (LSTM data), R-9 (Humble EOL horizon), R-5 (edge budget reality).

**Exit criteria:** tuning loop demo on SITL + bench FC with measured before/after; AI advisories carrying model version/confidence, apply/dismiss audit round-trip proven; `uaop.cmd.>` permission denial for ai-engine *tested*; ROS graph of a 20-node companion rendered live, reconciliation alarm demonstrated; SORA PDF for a reference ConOps validated against JARUS v2.5 by manual check; edge soak 24 h within resource budget (UAOP-NFR-003/005); five-layer validation green.

---

## Phase 3 — Simulation, digital twin, training

**Objective:** the simulation/validation substrate becomes product: scenarios, certification runner, twin, training scoring.

**Deliverables:** simulation-engine (orchestration, ≥ 10 scenario library incl. failure injections, seeded determinism); certification test runner with signed reports; digital twin T0/T1 (+T2 counterfactual if schedule holds — DIGITAL_TWIN.md §6 cut line); training scoring + instructor debrief views; 1000 Hz ingest benchmark met on RC-3 (ARM64); multi-vehicle: 50 SITL vehicles; SIM workspace; high-brightness field theme.

**Dependencies:** Phase 2 gate. **Risks:** R-8 (AirSim successor), R-10 (Gazebo/ROS matrix), R-2 (Timescale sizing on Jetson).

**Exit criteria:** scenario suite runs headless-parallel in CI nightly; a certification-runner report reviewed as evidence-grade by an external aviation engineer; twin divergence monitor catches an injected mass-change scenario; 50-vehicle SITL fleet stable 4 h; training scenario scored end-to-end with exportable record; five layers green.

---

## Phase 4 — Enterprise, SDK, certification readiness, cloud

**Objective:** commercially deployable: multi-tenant fleet, plugin SDK + marketplace, hardened security, full compliance package, cloud tier.

**Deliverables:** fleet-management engine + cloud tier (sync, org isolation, Next.js fleet dashboard — ADR-0002 scope); plugin SDK (three deliverables, SDK_DESIGN.md) + marketplace backend; UTM live integrations (LAANC, U-Space, Digital Sky); security hardening (mTLS mesh, FIPS build variant, HSM interface, SROS2 option, gateway runtime revisit per ADR-0004); DO-178C package completion (SVP, SQA records, SAS, 100% RTM); OTA at fleet scale; licensing per OQ-4.

**Dependencies:** Phase 3 gate; OQ-4/OQ-5 resolved; first design-partner customer identified (BUSINESS_MODEL.md).
**Risks:** R-3 (certification cost), R-7 (competitor response), R-12 (single-founder company risk at commercial stage).

**Exit criteria:** two-org tenancy demo with RBAC-scoped fleet ops over real sync; third-party-authored plugin passes marketplace pipeline and runs sandboxed; air-gap courier sync round-trip; penetration test by external party with findings dispositioned; DO-178C↔DO-278A package review by external DER-experienced consultant; a paying or committed design partner operating a pilot deployment.

---

## Sequencing rationale (why this order survives contact with reality)

Phase 1 builds the substrate every later phase consumes (contracts, audit, SITL discipline) — nothing in it is speculative. Phase 2 monetizes the platform's first *differentiating* depth (tuning + ROS) while AI stays advisory and cheap. Phase 3 converts internal validation muscle into product. Phase 4 spends money (cloud, security audits, certification consultants) only after the product has proven itself in Phases 1–3 demos — matching the zero-budget constraint that governs this company until revenue or funding changes it.
