# UAOP Development Guide (CLAUDE.md)

This file contains quick references for building, running, and testing the Unified Autonomy Operating Platform (UAOP).

## 🚀 Build & Run Commands

### Frontend Services

#### Next.js Web Dashboard (High-level telemetry & mapping)
- **Install**: `cd frontend/web-dashboard && npm install`
- **Dev Server**: `cd frontend/web-dashboard && npm run dev` (Runs on port 3000)
- **Production Build**: `cd frontend/web-dashboard && npm run build`

#### Next.js UI (Cesium, MapLibre & ROS2)
- **Install**: `cd frontend/ui-nextjs && npm install`
- **Dev Server**: `cd frontend/ui-nextjs && npm run dev` (Runs on port 3001)
- **Production Build**: `cd frontend/ui-nextjs && npm run build`

#### Qt Ground Control Station
- **Build**: `cd frontend/qt-desktop-gcs && qmake && make`

### Core Backend Services (Drogon C++ & Python AI/ROS2)
- **Docker Build & Run**: `docker-compose up --build` (Starts Drogon C++ server, Postgres, and Redis)
- **Individual Services**: Located in `backend/services/` (telemetry-engine, mission-engine, compliance-engine, etc.)

## ✈️ Autopilot & Simulation Submodules
Before running autopilot-connected workflows, ensure submodules are initialized:
```bash
git submodule update --init --recursive
```
- Autopilot source folders: `external/ardupilot/` and `external/qgroundcontrol/`

## ⚙️ Coding Standards & Safety Constraints
- **C++ Rules**: Must align with **MISRA C++:2023** standards. No dynamic memory allocation (`new` or `malloc`) during runtime. Use static memory pools.
- **TypeScript/React Rules**: Use strict types, avoid `any`. Use Zustand for state management.
- **MAVLink / DDS Protocols**: All vehicle telemetry must flow through MAVLink 2.0 or ROS2 Humble DDS bridges.
- **Flight Compliance**: All flight routes must validate airspace restrictions (FAA 400ft AGL, EASA SORA, DGCA NPNT).

