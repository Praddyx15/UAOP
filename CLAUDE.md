# CLAUDE.md — UAOP Development Guide

UAOP (Unified Autonomy Operating Platform) by Sixty Motion Aerospace: an operational platform sitting **above** PX4 / ArduPilot / MAVLink / ROS 2 / Gazebo. Not a GCS, not an autopilot — the integration layer is the product.

## Read order (before any work)

1. `README.md` — orientation and doc map
2. `docs/00_Project_Foundation/MASTER_CONTEXT.md` — the constitution; wins all conflicts
3. `docs/00_Project_Foundation/DECISIONS.md` — ADRs + Review Log + open questions (OQ)
4. The numbered doc area relevant to your task (`docs/00_…08_`)
5. `docs/07_Implementation_Guides/IMPLEMENTATION_PLAN.md` — current milestone (now: **M0**)

## Hard rules

- **Stack is locked** (ADR-0002/0005/0016): C++17 Drogon microservices · Python 3.11 FastAPI (AI/sim only) · Qt 6.8 LTS/QML GCS with C++20 backend · NATS JetStream · gRPC/Protobuf (`api/proto/` is the data constitution) · PostgreSQL 15 + TimescaleDB · Redis 7 · MinIO. No new technology without an ADR in DECISIONS.md.
- **Living design system**: after finishing any document or code milestone, run the seven-lens critical review (weaknesses, hidden assumptions, missing requirements, scalability, security, certification, tech debt) and record findings in DECISIONS.md's Review Log. Process: `docs/08_Developer_Handbook/DOCUMENTATION_PROCESS.md`. A doc that disagrees with code is a P1 defect.
- **Frozen directories**: `backend/services/dji-service/` and `backend/services/fpv-service/` are FROZEN pending OQ-1 — no code, contracts, or references.
- **Safety/compliance invariants**: AI output is advisory-only (ADR-0012); flight-influencing C++ follows MISRA C++:2023 with `@req:` traceability annotations, no exceptions/dynamic allocation on real-time paths (ADR-0013); commands require durable audit-event persistence (ADR-0015); air-gap operation is permanent — no Phase 1–3 cloud/SaaS dependencies.
- **Phase discipline**: work only inside the current milestone's scope (IMPLEMENTATION_PLAN.md). Scope creep is refused, not accommodated.

## Build (current prototype GCS — the only buildable code yet)

```
cmake -S frontend/qt-desktop-gcs -B frontend/qt-desktop-gcs/build
cmake --build frontend/qt-desktop-gcs/build
```
Requires Qt 6.5+ (target: 6.8 LTS). The prototype's view-per-page layout predates the dockable-panel architecture (`docs/04_UI_Architecture/UI_GUIDELINES.md`) and will be refactored in M0/M2 — treat it as a seed, not a pattern.

## History

Earlier planning artifacts (pre-baseline docs, original idea transcript, prototype Docker files, superseded tool configs) are archived in `docs/context/legacy/` — reference only, superseded by the numbered doc set.
