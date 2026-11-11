# BUSINESS_MODEL

**Version 0.2.0 · 2026-11-11 · ⚠ STATUS: ARCHITECT'S PROPOSAL. Pricing figures and packaging are NOT founder-validated (OQ-5); licensing posture is undecided (OQ-4). This document exists so commercial thinking constrains engineering (it already does — edge-first, air-gap, plugin walls) rather than being invented under sales pressure later.**

**Changelog:** v0.2.0 (ADR-0023) adds the verified price evidence (§2a), records the market evidence on OQ-4 (§4), and reorders go-to-market around the flight-evidence wedge and a demand checkpoint (§5). Prices below are still unvalidated.

## 1. What is actually sold

Not software features — **operational integrity**: one deployable platform replacing an integration project, with compliance evidence as a by-product and no cloud dependency to justify to a security office. The buyer's alternative isn't a competitor product; it's 6–18 months of internal glue work plus permanent maintenance (COMPETITOR_ANALYSIS.md §2's fragmentation tax).

## 2. Proposed packaging (maps to deployment tiers)

| Package | Contents | Buyer | Phase available |
|---|---|---|---|
| **UAOP Workstation** | Full single-seat platform + SITL, community support | Developers, labs, students | 1 — free (see §4) |
| **UAOP Edge Node** | Edge profile license per node: full ops stack, signed updates, support tier | UAV startups, OEMs, inspection operators | 2–3 |
| **UAOP Enterprise** | Multi-node + cloud tier: fleet, org RBAC, archive, sync; or fully on-prem enterprise variant | Enterprise/defense | 4 |
| **Compliance packs** | Jurisdiction data/logic subscriptions (rule updates as data — PLUGIN_SYSTEM.md §8) | All commercial tiers | 3+ |
| **Marketplace** | Rev-share on third-party plugins | Ecosystem | 4 |
| **Services** | Integration engagements, training-platform deployments, certification-evidence support | OEM/defense/institutes | opportunistic from 3 |

Proposed anchors (**OQ-5 — validate against real conversations before quoting anyone**): Edge Node node-and-support annual license in the low-thousands USD; Enterprise per-org + per-node; compliance packs low-hundreds annually per jurisdiction. India/DGCA pricing localized — the home-market advantage (Digital Sky integration nobody else prioritizes) should not be priced like a US defense contract.

## 2a. Price evidence found so far (2026-11-11)

Only two vendors' prices could be verified; neither is an Indian buyer's willingness to pay.

| Vendor | Published price | What it tells us |
|---|---|---|
| Foxglove (robotics log/visualisation) | Free ($0: 10 GB, 3 users, 5 devices) → Pro $20/month + $42 per developer seat + $20 per device + usage → Enterprise custom; **fully offline deployment only through a separate custom enterprise agreement** ([pricing](https://foxglove.dev/pricing), [primary sites](https://docs.foxglove.dev/docs/primary-sites)) | Open-core works in robotics tooling, and air-gapped is sold as the top tier, which is where UAOP's Edge Node sits |
| UgCS (mission planning) | Pro/Expert from $149/month or $890 lifetime ([SPH](https://www.sphengineering.com/flight-planning/ugcs/release-notes)) | A neutral PX4 + ArduPilot planning tool sells per seat at this level |

Auterion Suite prices that circulated (per-vehicle monthly figures) were refuted in verification and must not be quoted. No evidence was found for what Indian OEMs or DGCA-authorised RPTOs pay; that is what the M2E.6 checkpoint is for.

## 3. Licensing enforcement principle (engineering-binding, already honored)

**A license failure may never ground an aircraft.** Edge licensing is capability-gating at install/update time (signed bundles carry entitlements); there is no phone-home requirement to keep flying (CLOUD_ARCHITECTURE.md §5, air-gap axiom). Enforcement pressure lives in updates, support, and cloud features — the things a rational customer pays for anyway. This is both ethics and market fit: defense buyers red-line kill-switches.

## 4. OQ-4: the open-core question (the biggest open commercial decision)

| Option | For | Against |
|---|---|---|
| **Open-core** (Phase 1 core Apache-2.0; enterprise/compliance/fleet closed) | Community trust in an ecosystem-composition product; recruits plugin authors; FlightMD synergy; de-risks "aggregator abandonment" fear (COMPETITOR_ANALYSIS.md §3) | Gives incumbents free study; support burden before revenue |
| **Proprietary with free tiers** | Cleaner monetization; defense buyers indifferent to OSS | Cold-start ecosystem problem; harder trust story against QGC/MP's free gravity |

Architect's recommendation: **open-core**. Evidence added 2026-11-11: Foxglove runs exactly this ladder and charges separately for air-gapped deployment (§2a); SmartTune shows a free, agent-ready tuning tool appearing in the open (COMPETITOR_ANALYSIS.md). A natural split under ADR-0023: FlightMD and AirBrief cores open (they already are), UAOP's capture and evidence verifier open so anyone can check a bundle, and the Edge Node operations stack, observer-mode fleet history and compliance packs commercial. **Decision now due before the first design-partner agreement** (moved up from Phase 4). Until a LICENSE file exists the repository is all rights reserved. Founder decision required.

## 5. Go-to-market sequence (zero-budget-compatible; reordered by ADR-0023)

**Reordered 2026-11-11.** The earlier sequence (items 1–3 below) stays as the long arc, but the near term changes:

1. **Wedge, now to end of M2E:** FlightMD and AirBrief stay the free, offline funnel. UAOP's observer mode turns their output into a signed per-flight evidence bundle without asking anyone to change GCS.
2. **Demand checkpoint (M2E.6):** show that report and bundle, built from their own logs, to at least five Indian OEMs, RPTOs or integrators. Record what they would pay and what is missing.
3. **Company prerequisites, in parallel:** India's defence-innovation grants need an incorporated entity. ADITI (iDEX) accepts individual innovators but requires registration as a DPIIT-recognised startup or MSME before the grant agreement, co-funds up to 50% of the product budget (capped at Rs 25 crore) and expects the applicant to fund the rest ([scheme PDF](https://www.idex.gov.in/uploads/page/aditi_scheeme.pdf), [DDP page](https://ddpmod.gov.in/en/offerings/schemes-and-services/aditi_details)). Its FY2023–26 window has ended and no new call was found as of 2026-11-11, so incorporate and register before the next call rather than after.
4. **US/allied channel, later:** OMB M-26-02 judges civilian UAS buys on security controls, not on where software is written; the Blue UAS list now sits with DCMA. Whether a non-US software vendor can be listed is unverified, so plan on a US integrator partner. SECURITY_CONTROLS_MAP.md is the shared asset.

The original arc:

1. **Now–Phase 2:** FlightMD is the funnel — real users, forum credibility, the AI layer battle-tested in public (its own GTM plan: ArduPilot Discourse, PX4 community, LinkedIn, r/UAV, India UAV groups).
2. **Phase 2–3:** UAOP demo assets from validation gates (the tuning loop and SITL certification-runner demos *are* marketing); target 2–3 design partners: one India OEM (DGCA angle), one ROS-heavy robotics company, one training institute.
3. **Phase 4:** design partner converts to paying reference; defense conversations open with the air-gap + audit demo; marketplace seeds with our own plugins + academic-stack authors.

## 6. Cost structure honesty

Until Phase 4: near-zero cash (local infra, free CI tiers, Claude API cents-per-analysis via FlightMD; hardware per HARDWARE_ARCHITECTURE.md §6 in the hundreds). Phase 4 introduces real costs: cloud hosting, security audit, certification consultancy, and — the real one — additional humans (R-12). The business model must fund that transition; a design partner with NRE (non-recurring engineering) fees is the bridge candidate.

## 7. Business-model risks (cross-referenced)

R-12 (organizational depth for enterprise sales) · R-7 (funded competitor) · R-4 (founder bandwidth split with employment — the model must survive UAOP being nights-and-weekends for stretches) · OQ-4/OQ-5 unresolved. The engineering hedge for all four is the same: keep every phase's output independently valuable (a Phase 1 GCS, a Phase 2 tuning tool, a Phase 3 sim rig are each usable products), so the venture has offramps that aren't failures.
