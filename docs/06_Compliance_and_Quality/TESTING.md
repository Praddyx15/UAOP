# TESTING

**Version 0.1.1 · 2026-07-07 · How tests are built and executed. What must be proven per phase: VALIDATION.md. Pipeline wiring: CI_CD.md.**

## 1. Test pyramid (per repository area `tests/`)

| Level | Scope | Tooling | Targets |
|---|---|---|---|
| Unit | Domain logic in isolation (no framework, no I/O — the hexagonal core makes this cheap) | GoogleTest/Catch2 (C++), pytest (Python), Qt Test (GCS models) | DAL C-equiv: ≥ 90% line + **decision coverage** (the DAL C-appropriate objective — corrected per review R1/F6; MC/DC reserved for any future DAL A/B-equivalent scope); DAL D-equiv: ≥ 75% line |
| Contract | Every gRPC/NATS/proto contract: schema compatibility, envelope rules, error registry | buf breaking-change checks + generated round-trip tests | 100% of contracts, enforced on every PR |
| Integration | Service + real infrastructure (NATS, Postgres in containers) | testcontainers-style harness | Every service's consumer/producer paths incl. idempotency (duplicate-delivery tests are mandatory per EVENT_FLOW.md §3) |
| SITL / system | Full stack vs PX4/ArduPilot SITL | `tests/sitl/` harness (Python orchestration) | The VALIDATION.md matrix |
| Fault injection | Hostile conditions | harness kill/stall/corrupt primitives | VALIDATION.md fault suite |
| Fuzz | Zone 0 parsers | libFuzzer, structure-aware MAVLink corpus | mavlink-bridge continuous; gateway inputs |
| Compliance | RTM completeness, audit-chain verifier, annotation lint | `tests/compliance/` + rtm-generator self-checks | Gate-blocking |

## 2. Ground rules

1. **A bug fix without a regression test is an incomplete fix** — reviewer-enforced.
2. **Determinism:** flaky tests are quarantined within 24 h and fixed or deleted; SITL tests use lockstep and seeded scenarios; wall-clock sleeps are banned in favor of condition waits.
3. **Idempotency tests are first-class:** every NATS consumer gets a duplicate-delivery test and an out-of-order test (at-least-once semantics are only safe if actually tested).
4. **Absence testing:** proto3 optional discipline (TELEMETRY_ENGINE.md §2) is tested — a missing field must render as absent through the whole pipeline, never as zero.
5. **Test data is versioned:** sample logs (ULog/DataFlash including corrupt/truncated/short-flight edge cases mirroring FlightMD's hardened set), MAVLink corpora, and golden outputs live in-repo (LFS) with provenance notes. `tests/data/sample_logs/` now holds an initial clean/flawed fixture pair across all three formats (`.ulg`/`.bin`/`.tlog`), imported directly from FlightMD (Review R4/ADR-0018) — small enough (~1.5 MB) not to need LFS yet; a larger real-world corpus (50 flights, 11 vehicle types, ~8.5 GB) exists at the source project and is a deliberate future LFS-backed import, not yet scheduled.
6. **Coverage gates fail closed** per the targets above; DAL C-equiv modules additionally require review sign-off on any uncovered branch (justification recorded).

## 3. SITL harness design (`tests/sitl/`)

Python orchestration that: launches pinned-version SITL containers (PX4 + ArduPilot matrix from MAVLINK_INTEGRATION.md §2), brings up the platform (Compose profile), drives scenarios through the public API (gateway) exactly as the GCS would — *tests use no internal side doors, which keeps the API honest* — and asserts on both API responses and NATS/DB side effects (audit entries present, hashes chain, gaps accounted). Artifacts (telemetry extracts, logs) attach to CI runs for failure forensics.

## 4. Performance & soak testing

Benchmarks are tests with numeric assertions (fail on regression > 10% vs recorded baseline): 1000 Hz ingest throughput, hot-path latency budget segments (instrumented builds), cold-start time, GCS frame pacing under 10 Hz × N-vehicle load (Qt Quick profiler in CI headless mode where measurable, manual profile records otherwise). Soak (24 h) runs nightly-scheduled, not per-PR; memory-growth and message-loss assertions per UAOP-NFR-005.

## 5. GCS testing strategy

QML is notoriously undertested; our split makes it tractable: all logic lives in C++ models/viewmodels (unit-tested thoroughly, including staleness state machines and alert coalescing); QML views are covered by (a) `qmllint`/`qmlformat` gates, (b) a smoke harness instantiating every panel against recorded model fixtures (catches binding breaks), (c) scripted end-to-end flows on the workstation profile for the golden paths (connect → mission → fly SITL → review log). Visual/UX conformance stays a human review item with the UI_GUIDELINES.md checklist — pixel tests are brittle theater for a dense UI and we skip them deliberately.

## 6. What we intentionally do not test

Autopilot internals (PX4/ArduPilot correctness is upstream's job — we test our *handling* of their behavior including their failure modes); Gazebo physics fidelity (we test orchestration, injection, and evaluation); third-party plugin internals (we test the sandbox walls — a plugin failing inside its sandbox is the system *working*).
