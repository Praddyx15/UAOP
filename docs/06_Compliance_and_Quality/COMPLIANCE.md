# COMPLIANCE

**Version 0.1.0 · 2026-07-03 · Two distinct compliance domains, kept separate on purpose: (A) how we build the software (process assurance) and (B) what the platform does for the operator's regulatory obligations (product features).**

---

## A. Process assurance — how UAOP is built

### A.1 Framework and the honesty note

The constitution mandates **DO-178C-aligned** development. Formal note (ADR-0010): DO-178C governs airborne software; UAOP is ground software whose formal analog is **DO-278A/ED-109A**. We apply DO-178C process objectives and maintain a DO-178C↔DO-278A objective mapping table (to live at `docs/compliance/DO-178C/objective-mapping.md`) so the same artifacts serve either basis a customer or authority names. We **never claim certification** — we maintain certification *readiness* artifacts.

System-level concepts: **ARP4754A** (requirement development assurance — our HLR/LLR hierarchy and validation gates mirror its intent) and **ARP4761** (a platform-level FHA identifying how UAOP failures could contribute to hazards — e.g., "display of stale position as live" is our top-severity platform failure condition, which is why staleness handling appears in a dozen documents). **DO-330:** the RTM generator and the certification test runner are tool-qualification candidates the moment their output enters anyone's certification data; they are built with that trajectory (deterministic, versioned, self-checking output).

### A.2 Assurance levels

| Tier | Modules | Rigor |
|---|---|---|
| DAL C-equivalent | mavlink-bridge, vehicle-manager (state/authority/failsafe surfacing), mission-engine transfer/verify, parameter-engine write path, remote-id, compliance-engine chain | Full traceability, MISRA, no-exception/no-alloc RT rules, structural coverage targets, independent review |
| DAL D-equivalent | UI, analytics, logging, ai-engine, simulation, fleet | Traceability + standards, lighter verification depth |

### A.3 Plans and artifacts (produced as real documents under `docs/compliance/DO-178C/`)

PSAC, SDP, SVP, SCMP, SQA Plan during Phase 1; SAS at Phase 4. The RTM is generated from code annotations:

```cpp
// @req: UAOP-HLR-001  MAVLink ingestion and routing
// @req: UAOP-LLR-001-03  Parse HEARTBEAT and update vehicle state
// @test: UAOP-TC-001-A  Verify HEARTBEAT parsing in SITL
```

CI fails when DAL C-equivalent code lacks annotations (UAOP-NFR-009). Configuration management = git discipline + signed releases + the SCMP; QA = the phase-gate checklist discipline in VALIDATION.md.

---

## B. Product compliance features — what the platform does for operators

### B.1 Jurisdiction matrix (feature mapping)

| Feature | FAA | EASA | DGCA (India) |
|---|---|---|---|
| Remote ID | 14 CFR Part 89 / ASTM F3411-22a | EU 2019/945 Art. 47 / F3411 | Drone Rules 2021 R.18 / F3411 |
| Geo-awareness | LAANC / DroneZone data | U-Space geo-awareness | Digital Sky DAM (Green/Yellow/Red) |
| BVLOS authorization | Part 107 waiver / Part 135 | SORA (Specific category) | DGCA BVLOS permit |
| Risk assessment | ASTM F3269 MoC context | **JARUS SORA v2.5** (SAIL I–VI, iGRC, ARC, 24 OSOs) | DGCA SOP |
| UTM/USS | ASTM F3548-21 | EU 2021/664–666 | Digital Sky UTM |
| Altitude default | 400 ft AGL | 120 m AGL (Open) | 400 ft AGL |

### B.2 Feature set by phase

- **Phase 1:** hash-chained audit log + verifier + export (UAOP-HLR-042); 30-item pre-flight checklist gating AUTHORISE FLIGHT with signed operator identity (HLR-043); Remote ID monitoring/evidence (HLR-040); mission hash identity (HLR-012); jurisdiction adapters as stubs with **explicit NOT-CONNECTED/NOT-SUBMITTED states** (the platform never renders regulatory green it can't prove).
- **Phase 2:** SORA operations module — ConOps builder → automated SAIL calculation → OSO checklist (24 per JARUS v2.5) → iGRC/ARC display → signed PDF assessment. BVLOS permit tracker with expiry surfacing.
- **Phase 3:** compliance data packs (jurisdiction rules as versioned data — PLUGIN_SYSTEM.md §8); certification test runner reports designed to slot into evidence packages (SIMULATION_ARCHITECTURE.md §7).
- **Phase 4:** live UTM integrations (LAANC, U-Space service providers, Digital Sky API), DO-178C document package completion, archive/legal-hold in cloud tier.

### B.3 The audit chain (the load-bearing feature)

Append-only PostgreSQL, single writer, SHA-256 chain: `hash_n = SHA256(hash_{n-1} || canonical(event_n))`. Events (MASTER_CONTEXT.md §8 list) carry actor, vehicle, correlation, monotonic sequence. Verifier proves completeness and locates any break; export produces a self-verifying bundle. Fail-closed coupling (**refined by ADR-0015 after review R1/F1**): a vehicle command is admitted only once its audit event is durably persisted in the JetStream AUDIT stream; the Postgres chain append is asynchronous with monitored lag, and EMERGENCY commands survive even bus failure via a local WAL that reconciles into the chain on recovery. The guarantee that matters is preserved — the platform never acts without a durable record — without making a database restart capable of blocking an emergency RTL.

### B.4 What we deliberately do not do

No legal advice (the platform reports rule states, the operator holds responsibility — wording in UI is factual: "DAM zone: RED", never "you may not fly"); no auto-submission to regulators without explicit operator action; no synthetic compliance (sim data excluded from operational exports by provenance flag, DATA_FLOW.md §7).
