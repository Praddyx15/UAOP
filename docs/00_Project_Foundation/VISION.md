# VISION

**Version 0.1.0 · 2026-07-03**

## The ten-year picture

Every serious computing domain eventually produced a unifying platform layer. Desktops got operating systems. Game development got engines. Mobile got Android/iOS. UAV autonomy has not had that moment: it has world-class *components* — PX4 and ArduPilot for control, MAVLink for transport, ROS 2 for robotics middleware, Gazebo for physics — and no operational layer that makes them behave as one system.

**UAOP's vision is to be that layer: the operating platform for drone autonomy.**

Not by replacing the components — their communities, flight heritage, and hardware support are irreplaceable — but by sitting above them the way an OS sits above drivers: composing, supervising, recording, and presenting.

## What the world looks like if we succeed

- A UAV startup provisions a complete operations stack — GCS, tuning, simulation, logging, compliance — on one edge box in an afternoon, instead of integrating six tools over six months.
- A field operator runs BVLOS missions from a single console that shows the aircraft, the ROS 2 graph, the C2 links, and the regulatory state simultaneously — with no internet connection.
- A safety auditor receives a hash-chained, requirement-traced record of every parameter write, arm event, and AI recommendation ever issued — generated as a by-product of normal operation, not as an after-the-fact scramble.
- A researcher swaps the real aircraft for its digital twin and replays yesterday's anomaly against a candidate parameter set before touching hardware.

## Why now

1. **The stack is mature but the seams are not.** PX4/ArduPilot/ROS 2/Gazebo are each production-grade; the integration between them is still tribal knowledge and shell scripts.
2. **Regulation is forcing integration.** Remote ID (ASTM F3411-22a), SORA, and Digital Sky make compliance a runtime function, not paperwork. None of the incumbent tools treat it that way.
3. **Edge compute is cheap.** A Jetson-class device now runs the entire stack near the aircraft. The "cloud-first drone platform" architecture of the 2018–2022 generation is the wrong shape for defense and industrial buyers who demand air-gap.
4. **AI diagnostics finally work.** FFT/LSTM analysis of flight logs produces genuinely useful advisories — but only if the platform owns the full telemetry pipeline.

## What UAOP refuses to become

- **Not a cloud SaaS with an edge afterthought.** The edge node is the product; cloud is an amplifier.
- **Not a fork farm.** The moment we fork PX4 or QGC we inherit their maintenance burden and lose their upgrades. Adapters only.
- **Not a consumer drone app.** The user is a professional operator, engineer, or safety officer. Density and reliability beat polish and delight.
- **Not "AI-piloted."** AI advises; autopilots control; humans decide. This is both an ethical stance and a certification strategy.

## The moat, honestly stated

Individually, every UAOP feature exists somewhere. The moat is (a) the **integration itself** — correct, supervised interop across MAVLink/ROS 2/simulation is brutally hard and nobody has productized it; (b) **compliance-by-design** — audit trails and traceability retrofitted are worthless, ours are structural; (c) **air-gap-first** — a distribution posture cloud-native competitors cannot cheaply copy.

## Relationship to MISSION.md

This document says where we are going. [MISSION.md says what we do every quarter to get there, and the tests we apply to stay honest.
