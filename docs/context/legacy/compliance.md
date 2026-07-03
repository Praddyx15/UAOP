# UAOP Regulatory Compliance Matrix

As the Unified Autonomy Operating Platform (UAOP) handles safety-critical flight operations, all development, architecture, and CI/CD operations must align with the following global aviation and drone regulations.

## 1. Safety-Critical Software & Systems (DO-178C / ARP4754A)
*   **DO-178C (Software Considerations in Airborne Systems):** We target **DAL C/D** (Design Assurance Level) for drone software typically. This means strict traceability from requirements to code, automated unit testing, and measurable structural coverage (e.g., condition/decision coverage based on DAL).
*   **ARP4754A (Guidelines for Development of Civil Aircraft and Systems):** System-level architecture must have explicit functional hazard assessments (FHA) and structured fault-tree analysis (FTA) for catastrophic failures.

## 2. Telemetry & Identification
*   **Remote ID (FAA):** The ROS2-to-Frontend bridge must explicitly broadcast standardized Remote ID packets (UAV ID, Lat/Lon, Altitude, Velocity) over Bluetooth/WiFi broadcast formats or via network.
*   **UTM (UAS Traffic Management):** The Ground Control Station (GCS) map backend must support UTM API integration (e.g., pushing telemetry to an LAANC or U-Space provider).

## 3. Operational Rule Sets
*   **FAA Part 107 (US):** Constraints such as airspace ceiling limits (400ft AGL), speed limits (100 mph), and visual line of sight logic must be pre-programmed as soft fences in the mission planner.
*   **EASA UAS Regulations (EU):** Classification support for Open, Specific, and Certified categories.
*   **DGCA Rules (India):** NPNT (No Permission, No Takeoff) protocol integration inside the GCS to cryptographically verify flight paths before allowing arming.
*   **BVLOS Operations:** Requires independent redundant data links (e.g., SatCom + LTE) and rigorous lost-link Return-To-Launch (RTL) autonomous logic.

## 4. Drone Type Certification & SORA
*   **EASA: SC-VTOL, CS-UAS:** Compliance with vertical takeoff parameters and structural limits.
*   **FAA: AC 21-50:** Type certification documentation generation for the final integrated platform.
*   **JARUS SORA (Specific Operations Risk Assessment):** The mission planner must incorporate a preliminary ground risk and air risk assessment matrix tool prior to route submission.

## 5. Hardware/Parachute Integration
*   **ASTM F3269 (Means of Compliance):** Strict adherence to flight envelope restrictions and testing logs.
*   **ASTM F3322 (Small UAS Parachutes):** Explicit logic in the autopilot node to trigger parachute deployment upon unrecoverable attitude/altitude loss, simultaneously cutting motor power.
