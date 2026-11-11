# COMPETITOR_ANALYSIS

**Version 0.2.0 · 2026-11-11 · Refresh quarterly (R-7). Positioning truth (ADR-0023): several players now cover large slices, including air-gapped fleet operations and AI log diagnosis. Nobody verified covers the whole flight-evidence loop (brief, tune, fly, diagnose, prove) for both PX4 and ArduPilot.**

**Changelog:** v0.2.0 replaces the July desk analysis with two cited research passes (2026-11-11). Each claim was checked by three independent verifiers and kept only on a 2-of-3 vote. Where a row says *unverified*, the research found nothing solid either way. v0.1.0's claims that Auterion is "cloud-centric" and that air-gap is "not the design center" for anyone are withdrawn; FlytBase's row is corrected (it now ships air-gapped).

## 1. The landscape by slice

| Player | What it is (verified) | Where UAOP differs | Sources |
|---|---|---|---|
| **Auterion** | "Vendor-independent" drone OS (AuterionOS, Skynode), GCS (Mission Control), fleet and OTA updates (Auterion Suite), pre-flight checks, flight analysis for predictive maintenance, Remote ID. $130M Series B, Sep 2025, led by Bessemer, $25M non-dilutive backed by the US Department of War; valuation reported above $600M (anonymous source) and possibly ~$1.2B since (unconfirmed). | Vendor-independent means across **airframe makers**, on **PX4**: Skynode ships an Auterion-managed PX4 release, and no evidence of ArduPilot support was found. The best-funded competitor and the clearest opening. | [Auterion release 2025-09-23](https://auterion.com/auterion-raises-130-million-series-b-as-their-ai-enabled-software-powering-low-cost-commercial-hardware-at-scale-transforms-warfare/), [AuterionOS](https://auterion.com/product/auterion-os/), [PX4 Skynode docs](https://docs.px4.io/v1.17/en/companion_computer/auterion_skynode) |
| **FlytBase One** | Unified control platform (launched Feb 2026): docked and manual drones, ground robots, CCTV, sensors and counter-UAS in one system; cloud, on-premise or **fully air-gapped**; sold to defence and critical infrastructure. | **Air-gap alone no longer differentiates.** Hardware centres on DJI Docks; no PX4/ArduPilot, tuning, log diagnosis or DO-178C in its launch material. | [DroneLife 2026-02-18](https://dronelife.com/2026/02/18/flytbase-unveils-flytbase-one-management-system/), [FlytBase on-premise](https://flytbase.com/on-premise) |
| **Roboto** | Commercial log platform ($4.8M seed, 2023): auto-detects PX4 issues (vibration, GPS degradation), AI chat, agents, automated reports. | Direct overlap with FlightMD on PX4. ArduPilot DataFlash support **unverified** (§5). | [PX4 log analysis docs](https://docs.px4.io/main/en/log/flight_log_analysis) |
| **SmartTune CLI** | Open-source (MIT label), single author, created 2026-05: offline per-axis PID advice from step response, notch/low-pass suggestions from FFT, ±25% change cap, read-only MCP server for AI agents. ArduPilot full; PX4 ULog parsed but PX4 FFT/notch is a placeholder. No fleet features. | Closest open-source match to FlightMD's engine. **Behavioural reference only:** its ArduPilot analysis "replicates WebTools PIDReview.js" and WebTools is GPL-3.0 (ADR-0023 §7, ADR-0019 Tier 3). Self-reported capabilities, 33 stars, nobody has checked its tuning output. | [GitHub](https://github.com/raylanlin/smarttune-cli) |
| **PX4 log tools** | The PX4 docs list 11 tools: Flight Review, FlightPlot, PlotJuggler, MAVGCL, PX4Tools, pyulog, Data Comets, Foxglove, a VS Code ULog viewer, DroneBundle, Roboto. | Rich but disconnected from tuning, briefing and compliance. Flight Review is the de facto default. | [PX4 docs](https://docs.px4.io/main/en/log/flight_log_analysis) |
| **UgCS** (SPH Engineering) | Mission planning for any ArduPilot or PX4 drone over MAVLink, plus DJI; both autopilots maintained into Oct 2026 (v6.2.0 added ArduPilot 4.7.0). Pro/Expert from $149/month or $890 lifetime. | Neutral for **mission planning only**: SPH says it "does not replace Mission Planner for low-level ArduPilot parameter tuning". No log diagnosis, no audit chain. | [SPH article](https://www.sphengineering.com/news/how-to-use-ardupilot-px4-drones-with-ugcs-flight-planning-software-2024), [release notes](https://www.sphengineering.com/flight-planning/ugcs/release-notes) |
| **AirData UAV** | Fleet health and maintenance, cell-level battery analysis, mission planning and LAANC, pilot currency, 180+ drone types. | No PID/notch/vibration tuning on its feature page; PX4/ArduPilot not mentioned; one forum report of blank reports from ArduPilot `.bin` uploads. DJI-led. | [AirData features](https://airdata.com/features), [ArduPilot forum](https://discuss.ardupilot.org/t/ardupilot-flight-logs-into-airdata/88881) |
| **PDRL AeroGCS** (India) | GCS for agricultural drones (AeroGCS GREEN) and fleet tracking/logs (Enterprise); built on MAVLink. Vendor claims DO-278A AL5 IV&V (self-reported). | Indian civil/agri competitor at the GCS and fleet layers; no verified log diagnosis, tuning or air-gap. | [The Machine Maker 2024-11-14](https://themachinemaker.com/news/pdrl-lands-major-order-for-500-aerogcs-software-licenses-from-avpl-international/) |
| **AirMap** (closed) | Airspace/LAANC app; shut down 2023-06-22 after its 2021 acquisition by DroneUp. | Suggests compliance as a standalone product is fragile; UAOP keeps compliance as a feature of the evidence layer. | [DroneXL 2023-05-31](https://dronexl.co/2023/05/31/airmap-is-dead/) |
| **QGroundControl / Mission Planner** | Free open-source GCSs; QGC is PX4-first, Mission Planner has the deepest ArduPilot tuning. | Unchanged from v0.1.0: tools, not an evidence or audit layer. Under ADR-0023 UAOP works **alongside** them in observer mode (UAOP-HLR-033) instead of asking operators to switch. | — |

**Unverified in both passes** (no claim survived, so no position is asserted): Skydio, DroneDeploy, Aloft, Dronelink, Unifly, ANRA, Altitude Angel, Dedrone, Anduril Lattice, Shield AI Hivemind, Percepto, Ondas/American Robotics, Rerun, commercial QGC/Mission Planner forks, and the Indian players ideaForge, Garuda, Asteria, Aereo, Skylark.

## 2. The gap, stated precisely

The capabilities exist, split across tools: UgCS covers mixed-autopilot mission planning, AirData fleet maintenance, Roboto PX4 AI diagnosis, SmartTune ArduPilot tuning advice, FlytBase air-gapped fleet ops, Auterion a full PX4 stack. **No verified product combines PX4 and ArduPilot with tuning, log diagnosis and tamper-evident evidence in one place, offline.** That is UAOP's position under ADR-0023, built from three of the founder's products: AirBrief (pre-flight brief), FlightMD (diagnosis, tune-fly-verify) and UAOP (capture, audit chain, evidence bundle, edge ops).

This is an inference from absence across the tools checked, not a market-wide survey. §5 lists the checks that would overturn it.

## 3. Market signals (ranges, not points)

- **Drone software, global:** 2030 estimates run from about $11.9B (Lucintel) to $24.4B (Grand View: $9.27B in 2024, 16% CAGR); Grand View's own page is internally inconsistent. The narrower flight-control/GCS segment is about $1.2B in 2026 to $2.1B in 2031 (Knowledge Sourcing Intelligence, paywalled, low-tier). Sources: [Grand View](https://www.grandviewresearch.com/industry-analysis/drone-software-market-report), [KSI](https://knowledge-sourcing.com/report/drone-flight-control-software-market).
- **US defence:** the FY26 request broke out about $13.4B for autonomy and unmanned systems ($9.4B aerial), mostly platforms, not ops software; it is a request, not an appropriation. [DefenseScoop 2025-06-26](https://defensescoop.com/2025/06/26/dod-fy26-budget-request-autonomy-unmanned-systems/).
- **US civilian procurement:** OMB M-26-02 (2025-11-21) requires security controls "regardless of where the UAS was manufactured or assembled", including GCS remote-access MFA and update integrity; its bans target UAS hardware from covered foreign entities. The Blue UAS list moved from DIU to DCMA (bluelist.dcma.mil, Dec 2025 to Jan 2026). Mapped in [SECURITY_CONTROLS_MAP.md](../06_Compliance_and_Quality/SECURITY_CONTROLS_MAP.md).
- **India:** military drones have been barred from Chinese components since 2023 (tender clauses); the Army's AASHVAST lab (opened ~2026-08-14) tests drone firmware for hidden credentials, unsigned updates and spoofing response. No source puts GCS or ops software in its scope. The government has said it intends to extend the drone PLI and link it to R&D (Nov 2025; not notified). [The Week 2026-09-09](https://www.theweek.in/news/defence/2026/09/09/whats-hiding-inside-indias-military-drones-indian-armys-aashvast-to-eliminate-chinese-parts-security-risks.html), [ThePrint](https://theprint.in/defence/framework-to-restrict-chinese-components-in-military-drones-submitted-to-defence-ministry/2543584).

## 4. Honest counter-analysis (where we're weak)

- **Flight heritage: zero.** QGC/MP have decades of field debugging. Counter: observer mode lets UAOP run beside them, so trust can build without asking anyone to fly on UAOP first.
- **Funding gap.** Auterion can buy ArduPilot support if it decides the market is worth it. Counter: watch for it (R-7 trigger); UAOP's defensible part is the evidence discipline and offline verifiability, which venture speed handles worst.
- **Air-gap is matched.** FlytBase ships it. It stays a requirement (UAOP-NFR-001) but is no longer a headline.
- **Diagnosis is being commoditised.** Roboto, SmartTune and 11 PX4 tools. FlightMD must win on depth (PX4 and ArduPilot parity, validation against real logs, tune-fly-verify) and on being part of an evidence bundle, not on existing.
- **No demand evidence.** Nothing published shows what Indian OEMs or RPTOs pay. The M2E.6 checkpoint is the only fix.

## 5. Checks that would change this analysis

| Check | How | If positive |
|---|---|---|
| Does Roboto accept and diagnose ArduPilot DataFlash `.bin`? | Upload a `.bin` to Roboto's free tier | A neutral diagnosis competitor exists; R-7 fires; FlightMD's edge narrows to evidence and tune-fly-verify |
| Does Auterion Suite or Mission Control accept ArduPilot? | Free-tier upload and docs check | Same as above, with a funded competitor |
| Is AASHVAST or the Army Design Bureau framework extended to ground software? | Track DG EME / DAP 2026 publications | Indian defence becomes a formal market for SECURITY_CONTROLS_MAP.md evidence |
| Can a non-US software vendor be listed on the DCMA Blue UAS framework? | DCMA portal once reachable; a US integrator conversation | Opens a US/allied channel; otherwise sell through a partner |

## 6. Strategic implications baked into this doc set

Evidence, neutrality and offline verifiability lead the story (ADR-0023). Air-gap, audit chain and DO-178C-style traceability are the proof behind it, not headlines on their own. FlightMD and AirBrief are the wedge; UAOP's own GCS continues but stops being the pitch. ROS 2 depth, simulation and the fleet cloud stay later-phase, after demand is proven.
