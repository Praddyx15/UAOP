# PROJECT_STRUCTURE

**Version 0.1.4 · 2026-07-07 · The final, implementation-ready repository blueprint. Ownership rules and monorepo rationale: [REPOSITORY_STRUCTURE.md](../08_Developer_Handbook/REPOSITORY_STRUCTURE.md). ⊕ *directories* were created in the 2026-07-04 structure pass (review R2); ⊕ *files* (CMake presets, workflows, proto files, configs) land with milestone M0 code. ⚠ = frozen pending OQ-1.**

**Changelog:** v0.1.2 records two directories populated ahead of schedule from the real FlightMD project (Review R4 / ADR-0018): `backend/services/parameter-engine/data/` (seed metadata) and `tests/data/sample_logs/` (fixture logs) — both below, no longer bare `⊕`. v0.1.3 (M1.1): `api/proto/buf.yaml`/`buf.gen.yaml` and `api/proto/uaop/telemetry/v1/` landed for real, no longer bare `⊕`. v0.1.4 (M1.2): `api/proto/uaop/events/v1/envelope.proto` landed; `errors.proto` moved to a versioned `errors/v1/` directory (buf lint requirement) and landed for real.

## 1. Top-level tree (complete)

```
uaop-platform/
├── README.md
├── CMakePresets.json                ⊕ root presets: debug/release × x86_64/arm64 × sanitizer
├── .clang-format  .clang-tidy      ⊕
├── .github/workflows/               ⊕ pr.yml · nightly.yml · release.yml (CI_CD.md)
├── api/
│   └── proto/
│       ├── buf.yaml  buf.gen.yaml   — M1.1: v2 module, STANDARD lint, FILE breaking; C++/Python protoc_builtin codegen
│       └── uaop/
│           ├── telemetry/v1/        — M1.1: vehicle_telemetry.proto (13-category census) · snapshot.proto · delta.proto
│           ├── events/v1/           — M1.2: envelope.proto (EventEnvelope) landed; vehicle.proto · mission.proto · ... ⊕ per-domain, land with each engine (M2+)
│           ├── errors/v1/           — M1.2: errors.proto (ErrorCode registry + Error envelope) — versioned to satisfy buf's PACKAGE_DIRECTORY_MATCH lint rule, superseding the unversioned `errors.proto` path in API_SPECIFICATION.md §1
│           └── gateway/v1/          ⊕ command.proto · mission.proto · parameter.proto ·
│                                       telemetry_query.proto · log.proto · compliance.proto
├── backend/
│   ├── api-gateway/                 Node.js gRPC-Gateway (ADR-0004)
│   │   ├── src/{auth,routes,streams}/ ⊕   package.json  Dockerfile ⊕
│   └── services/
│       ├── common/                  ⊕ shared C++ lib (see §3 template note)
│       ├── telemetry-engine/        (§3 layout)
│       ├── mission-engine/
│       ├── parameter-engine/        ⊕ — `data/` seeded from FlightMD (px4_param_defaults, param_safe_ranges.json; R4/ADR-0018)
│       ├── vehicle-manager/         ⊕
│       ├── flight-log-engine/       ⊕
│       ├── remote-id/
│       ├── compliance-engine/       C++ core; sora-sidecar/ (Python, R1/F8 split)
│       ├── ai-engine/               Python (Phase 2)
│       ├── simulation-engine/       ⊕ Python/C++ (Phase 3)
│       ├── fleet-management/        ⊕ C++ (Phase 4)
│       ├── dji-service/             ⚠ FROZEN pending OQ-1 — no code
│       └── fpv-service/             ⚠ FROZEN pending OQ-1
├── middleware/
│   ├── mavlink-bridge/              ⊕ (OQ-2) C++ — §4 layout
│   └── ros2-bridge/                 C++ (Phase 2)
├── integrations/                    ⊕
│   ├── px4/                         message-definition pins · firmware metadata packs · SITL configs
│   ├── ardupilot/                   same
│   ├── utm/{laanc,u-space,digital-sky}/   (Phase 4 clients; stubs earlier)
│   └── remote-id/{broadcast,network}/
├── frontend/
│   └── qt-desktop-gcs/              §5 layout — Qt 6.8 LTS, C++20 (ADR-0016)
├── simulation/                      ⊕ gazebo/{worlds,models}/ · scenarios/ (Phase 3)
├── plugins/                         ⊕ sdk/{cpp,python,cli}/ · core-plugins/ (Phase 4)
├── compliance/
│   └── tools/                       ⊕ rtm-generator/ · misra-config/ · audit-log-exporter/
├── infrastructure/
│   ├── docker/compose/              compose.yaml · compose.sitl.yaml ⊕
│   ├── kubernetes/                  ⊕ base/ · overlays/{edge,edge-airgap,cloud}/
│   ├── terraform/                   ⊕ (Phase 4)
│   └── monitoring/                  ⊕ prometheus/rules/ · grafana/dashboards/
├── tests/
│   ├── sitl/                        harness/ ⊕ · suites/ ⊕ (VALIDATION.md matrix as code)
│   ├── integration/                 ⊕ per-service, containerized deps
│   ├── e2e/                         ⊕ golden-path GCS flows
│   ├── hil/                         ⊕ (Phase 2 bench)
│   ├── compliance/                  rtm/ · chain/ · annotations/ ⊕
│   └── data/                        sample_logs/ seeded from FlightMD (6 clean/flawed fixtures + generator, ~1.5MB, no LFS needed yet — R4/ADR-0018); larger corpora/goldens ⊕ LFS when needed
├── tools/                           ⊕ setup.sh · build-all.sh · uaop-node/ (ops CLI source)
├── docs/                            00_…08_ numbered living design system (this set)
│   ├── 00_Project_Foundation/ … 08_Developer_Handbook/
│   ├── api/ · architecture/ · scaling/ · context/phase-N/ (progress.md + validation evidence)
│   └── compliance/DO-178C/ ⊕ · SORA/ ⊕ · {FAA,EASA,DGCA}/ ⊕
└── external-resources/              reference-only; excluded from build/packaging/licensing
```

## 2. Placement invariants (mechanically checkable)

1. Only `middleware/*` speaks MAVLink/DDS; only `backend/services/*` owns DB schemas; only `api/proto/` defines cross-boundary types.
2. `frozen` directories (dji/fpv) contain a single FROZEN.md pointing at OQ-1 — CI fails on any other file until an ADR unfreezes them.
3. Generated code (proto stubs, QML type registration) never committed.
4. Every service directory contains: `README.md` (link to catalog entry), `CMakeLists.txt`/`pyproject.toml`, `src/`, `tests/`, `migrations/` (if DB-owning), `Dockerfile`.

## 3. Canonical C++ service layout (the template every engine follows)

```
backend/services/mission-engine/
├── README.md                        → catalog + engine doc links
├── CMakeLists.txt
├── Dockerfile                       multi-arch, distroless-runtime
├── migrations/                      0001_plans.sql …
├── src/
│   ├── main.cpp                     wiring only: config → adapters → app → serve
│   ├── domain/                      pure C++17, zero framework includes (MISRA scope)
│   │   ├── mission_plan.{h,cpp}     @req-annotated
│   │   ├── validators/              structural · dialect · geometric · energetic
│   │   └── geofence.{h,cpp}
│   ├── app/                         use-cases; audit-event emission points
│   │   ├── upload_mission.{h,cpp}
│   │   └── verify_deployment.{h,cpp}
│   ├── ports/                       abstract interfaces (VehicleLink, PlanRepo, EventBus)
│   └── adapters/
│       ├── grpc/                    handlers ← gateway
│       ├── nats/                    consumers/publishers (idempotent, dedup)
│       ├── postgres/                PlanRepo impl
│       └── bridge/                  VehicleLink gRPC client → mavlink-bridge
└── tests/
    ├── domain/                      fast, no I/O — the coverage-gate workhorse
    ├── app/                         use-case tests w/ fake ports
    └── adapters/                    integration (testcontainers)
```

`services/common/` provides: `Result<T,E>`, `EventEnvelope` helpers, JetStream client (durable+dedup pattern), layered config, structured logger, health/metrics scaffolding — and nothing domain-flavored (REPOSITORY_STRUCTURE.md rule 2 guard).

## 4. mavlink-bridge layout (deviates from template — RT constraints)

```
middleware/mavlink-bridge/
├── src/
│   ├── links/                       serial.cpp · udp.cpp · tcp.cpp (reader threads)
│   ├── rt/                          frame_pool.h · spsc_ring.h — no-alloc zone (ADR-0013)
│   ├── parser/                      generated tables from pinned XML + validators (fuzz target)
│   ├── translate/                   MAVLink ↔ VehicleTelemetry mapping, per-stack quirk tables
│   ├── transactions/                mission_transfer · param_io · command (protocol state machines)
│   ├── emergency_wal/               ADR-0015 local append log + reconciler (DAL C-equiv)
│   └── publish/                     NATS egress with bounded buffering + gap accounting
└── tests/  fuzz/                    libFuzzer harness + corpus
```

## 5. Qt GCS layout (state-of-the-art Qt 6.8: CMake + qt_add_qml_module, C++ models / QML views)

```
frontend/qt-desktop-gcs/
├── CMakeLists.txt                   qt_add_executable + qt_add_qml_module per panel
├── src/
│   ├── main.cpp
│   ├── core/                        AppContext · workspace manager · panel host/manifest loader
│   ├── net/                         GatewayClient (gRPC) · TelemetrySocket (WebSocket, I/O thread)
│   │                                C++20 coroutines for request flows (ADR-0016)
│   ├── models/                      one QAbstractItemModel/gadget family per stream:
│   │   ├── vehicle_model · telemetry_series · alert_stack · mission_model
│   │   ├── param_tree · checklist_model · audit_model · control_session_model (ADR-0017)
│   │   └── staleness.{h,cpp}        the shared freshness state machine (unit-tested hard)
│   ├── render/                      QQuickItem painters: efis · tapes · oscilloscope · node_graph
│   └── map/                         MapLibre Native integration (OQ-3 spike output)
├── ui/
│   ├── styles/Theme/                token singleton generated from styles/tokens.json
│   ├── components/                  ValueSlot · StalenessRing · CommandButton · AlertRow · Panel
│   ├── views/                       one QML module per panel (flight_hud/ map/ mission/ params/ …)
│   └── utils/
├── resources/                       fonts/Poppins/ (8 static faces, embedded via qt_add_resources; SIL OFL 1.1) · JetBrains Mono (pending) · icons
└── tests/                           models/ (Qt Test) · qml-smoke/ (fixture instantiation)
```

## 6. Naming and dependency direction (recap, enforceable)

kebab-case dirs; `uaop::<domain>` namespaces; proto packages `uaop.<domain>.v1` ≡ NATS subjects. Dependency arrows only point down the SOFTWARE_ARCHITECTURE.md §1 layers; the CI drift gate diffs the include/import graph against MICROSERVICES.md declarations (CI_CD.md §5) — this file is the tree those checks assume.
