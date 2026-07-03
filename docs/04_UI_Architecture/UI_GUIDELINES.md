# UI_GUIDELINES

**Version 0.1.0 · 2026-07-03 · Qt 6 / QML desktop GCS (ADR-0002). What the interface is and contains. How it behaves under operator load: UX_GUIDELINES.md. Visual tokens: STYLE_GUIDE.md.**

## 1. Design stance (constitutional)

Professional aerospace appearance. **Dense information, efficient workflow, no flashy animations.** Inspired by operational control rooms — not consumer software. The reference aesthetic is an EFIS/mission-console, dark, instrument-like: every pixel earns its place by carrying operational information or affordance. If a reviewer's reaction is "slick", we've drifted; the target reaction is "I can see everything".

## 2. Shell architecture: dockable panels, not tabs-as-pages

The Knowledge Base's 24-tab functional design is retained as the **functional census** but re-expressed as a **dockable panel workspace** (constitution: dense layout, dockable panels):

- A main window hosts a central surface (map/HUD) + dockable/tearable panels; panels are the 24 functional units re-cut where docking makes them stronger (e.g., IMU-Live docks beside Tuning during a tuning session).
- **Named workspaces** = saved layouts per activity: `FLIGHT OPS` (HUD, map, telemetry strips, checklist, alerts), `TUNING` (PID panels, oscilloscopes, FFT, param search), `ROS` (node graph, topic inspector), `SIM` (scenario console, injection panel), `REVIEW` (log graphs, findings, twin replay), `COMPLIANCE` (audit, SORA, permits). Workspaces switch atomically; operators build muscle memory against stable layouts.
- Multi-monitor: panels tear off natively — control-room reality is two or three displays.

## 3. Panel census (functional spec inherited from the 24-tab design)

Flight HUD (EFIS attitude, speed/alt tapes, heading tape, mode/arm controls, Remote ID status) · Map (MapLibre Native, vehicles, mission overlay, geofences, airspace layers) · Mission editor (waypoint list + property editor + stats) · Parameters (tree, search, group filter, versioned edit) · Sensors/calibration (12 cards) · Pre-flight checklist (30 items, gate to AUTHORISE) · Tuning (PID sliders, live response, AI advisor card) · Simulation console · AI diagnostics (health scores, findings) · Remote ID · Fleet cards · Compliance (SORA calculator, OSO checklist, audit viewer) · Logs/analysis · Radio calibration (16-channel live bars) · Motor test (**always-visible E-STOP**) · ROS 2 (node graph, topic inspector, lifecycle) · IMU live (3 oscilloscopes, vibration bars) · Vehicle config/airframe · Geofence manager · Swarm monitor · BVLOS (dual C2, permits, corridor) · Digital twin (3D, divergence panel) · Fail-safe config (6 sections, confirm-to-write) · Video/OSD · Node/system status (degradation ladder, disk forecast).

## 4. Information-density rules

- Numeric telemetry renders in fixed-width slots with units; values never jump position (layout stability beats compactness).
- Absence renders as `—` with a staleness ring, **never zero, never blank** (TELEMETRY_ENGINE.md §2 optionality carried to the glass).
- Every live value has a visible freshness state (fresh / stale-with-age / lost) — one visual language platform-wide (UX_GUIDELINES.md owns the semantics).
- Charts: gap regions drawn explicitly (hatched), sim/replay data watermarked, axes always labeled — a screenshot must be self-explanatory as evidence.
- Density scales: a compact mode for 13″ field laptops and a control-room mode; both from the same layout definitions, no separate "mobile-ish" UI.

## 5. Motion policy

Animations only when they encode information: smooth attitude/tape movement (instrument continuity), state-change cross-fades ≤ 150 ms, alert onset per UX rules. No decorative easing, parallax, bounce, or celebratory anything. The EFIS pair (attitude ball, tapes) targets 60 fps; everything else may update at data rate.

## 6. Technical construction rules

- QML views bind to **C++ models only** (`QAbstractItemModel`/typed gadgets); zero business logic in QML — JavaScript in QML confined to trivial view glue (testability split, TESTING.md §5).
- Panel = QML module with a manifest (id, models required, min sizes, workspace hints) — the same shape plugin panels use (PLUGIN_SYSTEM.md §5), so core and plugin panels are one mechanism.
- Rendering: Qt Quick scene graph; custom `QQuickItem`s for EFIS, tapes, oscilloscopes, node graph (canvas-heavy widgets get purpose-built painters, not stacked primitives).
- Update coalescing at the model layer to display rate (SOFTWARE_ARCHITECTURE.md §4) — the UI thread never chases a 1 kHz stream.
- All strings via Qt translation units from day one (DGCA market implies future localisation; retrofitting tr() is misery).
- Keyboard-first: every operator action reachable without mouse; shortcuts stable across workspaces (UX_GUIDELINES.md owns the map).

## 7. The command surface rule (safety-critical UI law)

Controls that command the vehicle (ARM, mode, RTL, motor test, fail-safe writes, HIGH-risk params) follow one platform-wide pattern: distinct visual class (STYLE_GUIDE.md command styling) · never adjacent to benign controls · confirmation semantics scale with risk class (TUNING_ENGINE.md §4) · disabled states always explain themselves ("ARM unavailable: checklist 22/30") · **disabled wholesale in replay/sim-review contexts** (DIGITAL_TWIN.md §4). Plugin panels cannot host command controls at all (PLUGIN_SYSTEM.md §3).
