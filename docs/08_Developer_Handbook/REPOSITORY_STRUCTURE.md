# REPOSITORY_STRUCTURE

**Version 0.1.1 · 2026-07-04 · Monorepo layout, ownership, and the rules that keep it navigable. The full directory tree exists as of the 2026-07-04 structure pass (review R2); ⊕ marks entries whose *files* land with M0 code; ⚠ = frozen pending OQ-1. File-level blueprint: [PROJECT_STRUCTURE.md](../07_Implementation_Guides/PROJECT_STRUCTURE.md).**

## 1. Layout

```
uaop-platform/
├── README.md
├── .github/workflows/            ⊕ CI pipelines (CI_CD.md) — treated as DAL D-equiv code
├── api/
│   └── proto/                    Data constitution: telemetry/, events/, gateway/, errors.proto
│                                 Versioned packages (…/v1/); buf-managed; breaking-change-gated
├── backend/
│   ├── api-gateway/              Node.js gRPC-Gateway (ADR-0004)
│   └── services/
│       ├── common/               Shared C++ library: Result<T,E>, envelope, NATS/gRPC helpers,
│       │                         config loader — the ONLY cross-service code-sharing point
│       ├── telemetry-engine/     C++ (TELEMETRY_ENGINE.md)
│       ├── mission-engine/       C++ (MISSION_ENGINE.md)
│       ├── parameter-engine/     ⊕ C++ (OQ-2 sibling; constitution §7)
│       ├── vehicle-manager/      ⊕ C++
│       ├── flight-log-engine/    ⊕ C++
│       ├── remote-id/            C++
│       ├── compliance-engine/    C++/Python
│       ├── ai-engine/            Python (AI_ENGINE.md; consumes flightmd_core)
│       ├── simulation-engine/    ⊕ Python/C++ (Phase 3)
│       ├── fleet-management/     ⊕ C++ (Phase 4)
│       ├── dji-service/          ⚠ frozen pending OQ-1 — no code until ADR
│       └── fpv-service/          ⚠ frozen pending OQ-1
├── middleware/
│   ├── mavlink-bridge/           ⊕ C++ (OQ-2 — recommend create; Phase 1 critical)
│   └── ros2-bridge/              C++ (Phase 2)
├── integrations/                 ⊕ Ecosystem adapters that aren't long-running services:
│   ├── px4/ ardupilot/           firmware-version metadata packs, SITL configs
│   ├── utm/                      laanc/ u-space/ digital-sky/ clients
│   └── remote-id/                broadcast/network module integration
├── frontend/
│   └── qt-desktop-gcs/
│       ├── src/                  C++: models, viewmodels, net clients, panel host
│       └── ui/                   QML: views/ components/ styles/ (theme singleton) utils/
├── simulation/                   ⊕ gazebo/ worlds+models, scenarios/ library (Phase 3)
├── plugins/                      ⊕ sdk/ + core-plugins/ (Phase 4; contracts earlier)
├── compliance/
│   └── tools/                    ⊕ rtm-generator/, misra-config/, audit-log-exporter/
├── infrastructure/
│   ├── docker/compose/           Workstation profile
│   ├── kubernetes/               ⊕ Kustomize base/ + overlays/{edge,edge-airgap,cloud}
│   ├── terraform/                ⊕ Cloud (Phase 4)
│   └── monitoring/               ⊕ Prometheus rules, Grafana dashboards (OBSERVABILITY.md §4)
├── tests/
│   ├── sitl/                     System harness (TESTING.md §3)
│   ├── compliance/               RTM/chain/annotation gates
│   ├── integration/ e2e/ hil/    ⊕
│   └── data/                     ⊕ versioned logs/corpora/goldens (LFS)
├── tools/                        ⊕ setup.sh, uaop-node CLI source, dev scripts
├── docs/                         This documentation set (flat, + subdirs below)
│   ├── architecture/             Deep-dive artifacts, diagrams source
│   ├── compliance/               ⊕ DO-178C/ (plans, RTM, objective-mapping) SORA/ FAA/ EASA/ DGCA/
│   ├── api/                      Generated API reference
│   ├── context/                  Phase validation evidence, progress.md per phase
│   └── scaling/                  Capacity/benchmark records
└── external-resources/           Reference material only — NEVER imported/linked by build
                                  (ecosystem repo studies per Master Context; keep out of packaging)
```

## 2. Rules that keep a monorepo healthy

1. **Monorepo, one exception:** `flightmd_core` stays in the FlightMD repo, consumed as a pinned pip dependency (ADR-0014) — it has its own users and release cadence.
2. **Service directories are sovereign:** own `CMakeLists.txt`/`pyproject.toml`, own `migrations/`, own tests co-located, own README (purpose + catalog link). Cross-service C++ sharing only via `services/common/` (its API reviewed like a public contract — the common-library-becomes-a-monolith failure mode is reviewed against explicitly).
3. **`api/proto/` is the only shared truth:** generated code is build-time output, never committed.
4. **CODEOWNERS mirrors criticality:** DAL C-equiv paths (mavlink-bridge, vehicle-manager, mission/parameter transfer paths, remote-id, compliance-engine) require two reviews (CI_CD.md §1).
5. **`external-resources/` hygiene:** study material excluded from builds, packaging, licensing scans, and release artifacts (its licenses are not our licenses). Git-ignored except its README (rules live there); UAV/robotics reference repos only. The unrelated agent-tooling content that once lived here was removed 2026-07-04 (review R2/F15).
6. **Naming:** kebab-case directories/services; proto packages `uaop.<domain>.v1`; NATS subjects mirror proto packages (EVENT_FLOW.md §1); C++ namespaces `uaop::<domain>`.

## 3. Build orchestration

CMake presets at repo root build any C++ subset; `uv`/pip workspaces for Python; one `tools/build-all.sh` honored by CI (a contributor can always reproduce CI locally — non-negotiable for debugging gate failures). Container builds per service from a shared minimal base image family (x86_64+ARM64 multi-arch, CI_CD.md §2).

## 4. Documentation placement rule

Flat `docs/*.md` = platform-level, stable, cross-referenced (this set); `docs/<subdir>/` = deep or generated artifacts; service-local READMEs = orientation only, linking upward — never duplicating (duplicated docs fork and rot; MISSION.md makes doc-code disagreement a P1 defect, so single-sourcing is the survival strategy).
