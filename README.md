# UAOP — Unified Autonomy Operating Platform

**Sixty Motion Aerospace** · Status: **Pre-implementation baseline (Phase 0)** · Doc set: **v0.1.0 · 2026-07-03**

UAOP is an operational platform that sits **above** existing UAV autonomy ecosystems — PX4, ArduPilot, MAVLink, ROS 2, Gazebo — and unifies ground control, deep tuning, simulation, AI diagnostics, fleet operations, and regulatory compliance into one coherent, deployable edge-to-cloud product.

UAOP is **not** another ground control station and **not** another autopilot. It does not fork or replace the open ecosystems it integrates; it composes them through adapters and presents a single operational surface.

---

## What problem does UAOP solve?

Professional UAV operations today require six or more disconnected tools: a GCS (QGroundControl / Mission Planner), tuning workflows buried inside the GCS, ROS 2 introspection (RViz/rqt), simulation (Gazebo/SITL), log analysis (Flight Review), and ad-hoc fleet/compliance tooling. Each works alone; none work together. UAOP's thesis: **the integration layer is the product.**

## Deployment model

| Tier | Purpose | Phase |
|---|---|---|
| **Workstation** | Development, simulation, training | 1 |
| **Edge Node** | Field operations near the aircraft — the primary product | 1–3 |
| **Cloud** | Fleet analytics, multi-site enterprise, compliance archive | 4 |

The Edge Node must operate fully **without internet connectivity** (air-gap mode is a supported deployment configuration, not a degraded one).

## Technology spine (fixed — see [DECISIONS.md](docs/00_Project_Foundation/DECISIONS.md) before proposing changes)

- **Services:** C++17 microservices (Drogon) · Python 3.11 (FastAPI) for AI/simulation orchestration
- **Desktop GCS:** Qt 6 / QML — professional aerospace UI, dense information, dockable panels
- **Messaging:** NATS JetStream internal bus · MAVLink v2 · ROS 2 (DDS) · gRPC · WebSocket
- **Data:** PostgreSQL 15 + TimescaleDB (telemetry to 1000 Hz) · Redis 7 · MinIO
- **Simulation:** Gazebo + PX4 SITL / ArduPilot SITL
- **Compliance:** DO-178C-aligned processes, MISRA C++, requirements traceability from day one

## Documentation map

All engineering documentation lives in [`docs/`](docs/) as a **living design system** in nine numbered areas (`00_Project_Foundation` → `08_Developer_Handbook`), evolved incrementally with a mandatory critical review after every document — see [DOCUMENTATION_PROCESS](docs/08_Developer_Handbook/DOCUMENTATION_PROCESS.md). Start here:

| Area | Documents |
|---|---|
| Foundation | [MASTER_CONTEXT](docs/00_Project_Foundation/MASTER_CONTEXT.md) · [VISION](docs/00_Project_Foundation/VISION.md) · [MISSION](docs/00_Project_Foundation/MISSION.md) · [PRODUCT_REQUIREMENTS](docs/00_Project_Foundation/PRODUCT_REQUIREMENTS.md) |
| Architecture | [SYSTEM_ARCHITECTURE](docs/02_System_Architecture/SYSTEM_ARCHITECTURE.md) · [SOFTWARE_ARCHITECTURE](docs/03_Software_Architecture/SOFTWARE_ARCHITECTURE.md) · [HARDWARE_ARCHITECTURE](docs/02_System_Architecture/HARDWARE_ARCHITECTURE.md) · [EDGE_ARCHITECTURE](docs/02_System_Architecture/EDGE_ARCHITECTURE.md) · [CLOUD_ARCHITECTURE](docs/02_System_Architecture/CLOUD_ARCHITECTURE.md) · [MICROSERVICES](docs/03_Software_Architecture/MICROSERVICES.md) |
| Flows & extension | [DATA_FLOW](docs/02_System_Architecture/DATA_FLOW.md) · [EVENT_FLOW](docs/02_System_Architecture/EVENT_FLOW.md) · [PLUGIN_SYSTEM](docs/01_Product_Architecture/PLUGIN_SYSTEM.md) · [SDK_DESIGN](docs/01_Product_Architecture/SDK_DESIGN.md) · [API_SPECIFICATION](docs/03_Software_Architecture/API_SPECIFICATION.md) |
| Integrations | [MAVLINK_INTEGRATION](docs/02_System_Architecture/MAVLINK_INTEGRATION.md) · [ROS2_INTEGRATION](docs/02_System_Architecture/ROS2_INTEGRATION.md) · [SIMULATION_ARCHITECTURE](docs/02_System_Architecture/SIMULATION_ARCHITECTURE.md) · [DIGITAL_TWIN](docs/01_Product_Architecture/DIGITAL_TWIN.md) |
| Engines | [MISSION_ENGINE](docs/01_Product_Architecture/MISSION_ENGINE.md) · [TELEMETRY_ENGINE](docs/01_Product_Architecture/TELEMETRY_ENGINE.md) · [TUNING_ENGINE](docs/01_Product_Architecture/TUNING_ENGINE.md) · [AI_ENGINE](docs/01_Product_Architecture/AI_ENGINE.md) · [FLEET_MANAGEMENT](docs/01_Product_Architecture/FLEET_MANAGEMENT.md) |
| Operations | [DEPLOYMENT](docs/05_Deployment_Architecture/DEPLOYMENT.md) · [SECURITY](docs/06_Compliance_and_Quality/SECURITY.md) · [COMPLIANCE](docs/06_Compliance_and_Quality/COMPLIANCE.md) · [VALIDATION](docs/06_Compliance_and_Quality/VALIDATION.md) · [TESTING](docs/06_Compliance_and_Quality/TESTING.md) · [CI_CD](docs/05_Deployment_Architecture/CI_CD.md) · [DEVOPS](docs/05_Deployment_Architecture/DEVOPS.md) · [DATABASE](docs/03_Software_Architecture/DATABASE.md) · [LOGGING](docs/05_Deployment_Architecture/LOGGING.md) · [OBSERVABILITY](docs/05_Deployment_Architecture/OBSERVABILITY.md) |
| UI & standards | [UI_GUIDELINES](docs/04_UI_Architecture/UI_GUIDELINES.md) · [UX_GUIDELINES](docs/04_UI_Architecture/UX_GUIDELINES.md) · [STYLE_GUIDE](docs/04_UI_Architecture/STYLE_GUIDE.md) · [REPOSITORY_STRUCTURE](docs/08_Developer_Handbook/REPOSITORY_STRUCTURE.md) · [CODING_STANDARDS](docs/08_Developer_Handbook/CODING_STANDARDS.md) |
| Governance | [DECISIONS](docs/00_Project_Foundation/DECISIONS.md) · [ROADMAP](docs/00_Project_Foundation/ROADMAP.md) · [RISK_REGISTER](docs/00_Project_Foundation/RISK_REGISTER.md) · [COMPETITOR_ANALYSIS](docs/00_Project_Foundation/COMPETITOR_ANALYSIS.md) · [BUSINESS_MODEL](docs/00_Project_Foundation/BUSINESS_MODEL.md) |
| Implementation | [IMPLEMENTATION_PLAN](docs/07_Implementation_Guides/IMPLEMENTATION_PLAN.md) · [PROJECT_STRUCTURE](docs/07_Implementation_Guides/PROJECT_STRUCTURE.md) · [DOCUMENTATION_PROCESS](docs/08_Developer_Handbook/DOCUMENTATION_PROCESS.md) |

## Governance rules

1. [MASTER_CONTEXT.md](docs/00_Project_Foundation/MASTER_CONTEXT.md) is the constitution. On conflict, it wins; conflicts are resolved only through an ADR in [DECISIONS.md](docs/00_Project_Foundation/DECISIONS.md).
2. No technology enters the stack without an ADR stating the engineering reason and the rejected alternatives.
3. Flight-critical modules carry `@req:` traceability annotations from the first commit — traceability is never retrofitted.
4. AI output is **advisory only**: confidence-scored, audit-logged, operator decides.
5. Air-gap operation is a permanent requirement. No service may hard-depend on an external network.

## Repository layout (summary)

```
uaop-platform/
├── api/proto/            # Protobuf + gRPC contracts (the platform's data constitution)
├── backend/
│   ├── api-gateway/
│   └── services/         # telemetry-engine, mission-engine, ai-engine, compliance-engine, remote-id, ...
├── middleware/           # mavlink-bridge, ros2-bridge
├── frontend/qt-desktop-gcs/
├── infrastructure/docker/compose/
├── tests/                # unit, integration, sitl, compliance
└── docs/                 # everything linked above
```

Full rationale: [REPOSITORY_STRUCTURE.md](docs/08_Developer_Handbook/REPOSITORY_STRUCTURE.md).

---

*This repository is in the documentation-baseline stage. Implementation begins only after the Phase 1 validation gate in [ROADMAP.md](docs/00_Project_Foundation/ROADMAP.md) is agreed.*
