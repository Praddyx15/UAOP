# BUSINESS_MODEL

**Version 0.1.0 · 2026-07-03 · ⚠ STATUS: ARCHITECT'S PROPOSAL. Pricing figures and packaging are NOT founder-validated (OQ-5); licensing posture is undecided (OQ-4). This document exists so commercial thinking constrains engineering (it already does — edge-first, air-gap, plugin walls) rather than being invented under sales pressure later.**

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

## 3. Licensing enforcement principle (engineering-binding, already honored)

**A license failure may never ground an aircraft.** Edge licensing is capability-gating at install/update time (signed bundles carry entitlements); there is no phone-home requirement to keep flying (CLOUD_ARCHITECTURE.md §5, air-gap axiom). Enforcement pressure lives in updates, support, and cloud features — the things a rational customer pays for anyway. This is both ethics and market fit: defense buyers red-line kill-switches.

## 4. OQ-4: the open-core question (the biggest open commercial decision)

| Option | For | Against |
|---|---|---|
| **Open-core** (Phase 1 core Apache-2.0; enterprise/compliance/fleet closed) | Community trust in an ecosystem-composition product; recruits plugin authors; FlightMD synergy; de-risks "aggregator abandonment" fear (COMPETITOR_ANALYSIS.md §3) | Gives incumbents free study; support burden before revenue |
| **Proprietary with free tiers** | Cleaner monetization; defense buyers indifferent to OSS | Cold-start ecosystem problem; harder trust story against QGC/MP's free gravity |

Architect's recommendation: **open-core**, decided no later than Phase 4 planning but *announced* intent earlier — the community window (FlightMD momentum, forum presence) is time-limited. Founder decision required.

## 5. Go-to-market sequence (zero-budget-compatible)

1. **Now–Phase 2:** FlightMD is the funnel — real users, forum credibility, the AI layer battle-tested in public (its own GTM plan: ArduPilot Discourse, PX4 community, LinkedIn, r/UAV, India UAV groups).
2. **Phase 2–3:** UAOP demo assets from validation gates (the tuning loop and SITL certification-runner demos *are* marketing); target 2–3 design partners: one India OEM (DGCA angle), one ROS-heavy robotics company, one training institute.
3. **Phase 4:** design partner converts to paying reference; defense conversations open with the air-gap + audit demo; marketplace seeds with our own plugins + academic-stack authors.

## 6. Cost structure honesty

Until Phase 4: near-zero cash (local infra, free CI tiers, Claude API cents-per-analysis via FlightMD; hardware per HARDWARE_ARCHITECTURE.md §6 in the hundreds). Phase 4 introduces real costs: cloud hosting, security audit, certification consultancy, and — the real one — additional humans (R-12). The business model must fund that transition; a design partner with NRE (non-recurring engineering) fees is the bridge candidate.

## 7. Business-model risks (cross-referenced)

R-12 (organizational depth for enterprise sales) · R-7 (funded competitor) · R-4 (founder bandwidth split with employment — the model must survive UAOP being nights-and-weekends for stretches) · OQ-4/OQ-5 unresolved. The engineering hedge for all four is the same: keep every phase's output independently valuable (a Phase 1 GCS, a Phase 2 tuning tool, a Phase 3 sim rig are each usable products), so the venture has offramps that aren't failures.
