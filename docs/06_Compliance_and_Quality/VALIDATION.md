# VALIDATION

**Version 0.1.0 · 2026-07-03 · The five-layer validation framework applied at every phase gate. TESTING.md covers *how tests run*; this document covers *what must be proven*.**

## 1. The five layers (constitutional)

Every phase exit requires all five layers green, with evidence archived per phase under `docs/context/phase-N-validation/`:

| Layer | Question answered | Evidence form |
|---|---|---|
| **1. Requirements** | Are the phase's requirements complete, unambiguous, testable, and traced? | Requirement review record; RTM slice showing HLR→LLR→TC coverage for the phase scope |
| **2. Architecture** | Does the implementation match the documented architecture? Invariants intact? | Invariant checklist (SYSTEM_ARCHITECTURE.md §6) walked against the code; dependency-graph diff vs MICROSERVICES.md; ADR audit (no undocumented tech) |
| **3. Code** | Standards, static analysis, review discipline | MISRA/clang-tidy/Cppcheck zero-critical report; coverage report vs targets; review log |
| **4. Integration** | Do the parts work as a system under realistic and hostile conditions? | SITL validation matrix results; fault-injection suite results; soak/benchmark records (NFR evidence) |
| **5. Compliance** | Are assurance artifacts current and the product compliance features truthful? | RTM generation clean; audit-chain verification; plan documents updated; security review record |

A layer with findings can conditionally pass **only** via a written waiver in DECISIONS.md (what, why, remediation date) — silent waivers don't exist.

## 2. Requirements validation discipline (layer 1 detail)

- Every HLR has: verification method tag (T/A/I/D), at least one LLR or a rationale for direct verification, and an owner.
- Ambiguity hunting is explicit: requirements containing "appropriately", "quickly", "robust" without numbers are rejected in review (the NFR table's numeric style is the standard).
- Change control: requirement edits after a phase gate re-open the affected trace slice — the RTM generator diffs and flags orphaned tests/annotations.

## 3. Integration validation matrix (layer 4 detail — the heart of a UAV platform's credibility)

Run at every gate, expanded per phase:

| Suite | Contents (Phase 1 baseline) |
|---|---|
| **SITL functional** | PX4 + ArduPilot: connect/reconnect, telemetry rates (10 Hz full-state, 50 Hz attitude), 100-item mission round-trip with byte verify, full param table read + verified writes, geofence upload + breach surfacing, Remote ID cycle evidence, log download |
| **Fault injection** | Kill each service mid-operation (UAOP-NFR-004); NATS restart under load; DB stall during ingest; link loss mid-mission-transfer; malformed MAVLink corpus; disk-watermark forcing |
| **Multi-vehicle** | 5 simultaneous SITL vehicles (Phase 1) → 50 (Phase 3): routing integrity, no cross-vehicle state bleed, UI fleet view correctness |
| **Disconnect test** | Full air-gap run: WAN severed, entire functional suite must pass except sync/USS indicators (UAOP-NFR-001) |
| **Soak** | 24 h simulated ops: zero JetStream-persisted loss, memory-flat, gap accounting exact (UAOP-NFR-005) |
| **Performance** | Latency budget verification per DATA_FLOW.md §3; 1000 Hz ingest benchmark; cold-start ≤ 120 s |

## 4. Validation responsibilities under tool-accelerated development

Because code volume can grow quickly when drafting is tool-accelerated (MASTER_CONTEXT.md §11), validation carries extra weight and specific counter-measures: all code merges only through the same review + static-analysis + trace gates regardless of how it was drafted (no "the tool said it passes"); each phase's `progress.md` records integration status and review findings honestly; architecture-layer validation specifically hunts for **drift** (helpers that bypass documented seams — e.g., a service quietly opening another's DB) because that is the characteristic failure mode of fast-drafted code at scale.

## 5. Hardware validation (Phases 2–3 onward)

Bench: real FC (Pixhawk-class) on USB — timing/behavior deltas vs SITL recorded; radio-in-loop: RFD900-class link with induced degradation (attenuation, interference) validating link-quality adaptation and loss handling; reference-hardware soak on RC-2/RC-3 for the resource-budget NFRs. Real flight validation happens only under the founder's DGCA licensing and applicable rules, with the platform in observe-record roles until bench maturity earns command roles — the validation program itself follows the safety posture the product preaches.

## 6. Traceability of validation itself

Gate results are signed records (who ran, versions, artifacts) stored with the phase evidence — the validation history is an auditable asset (and, practically, the demo that closes defense conversations).
