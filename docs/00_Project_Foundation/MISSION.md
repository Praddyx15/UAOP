# MISSION

**Version 0.1.0 · 2026-07-03**

## Mission statement

**Build the unified operational platform above open UAV autonomy ecosystems — deployable at the edge, honest about safety, and engineered so certification is a milestone rather than a rewrite.**

## What we do (and the test for each)

| Commitment | Operational test |
|---|---|
| Integrate, never fork | Zero forked ecosystem repos in our dependency tree. Adapters live in `middleware/` and `integrations/` with pinned upstream versions. |
| Edge-first reliability | Every release candidate passes the disconnect test: pull the WAN cable, fly a full SITL mission, all functions except cloud sync succeed. |
| Compliance as a by-product | An auditor can export the complete, gap-free hash-chained audit log for any flight without engineering assistance. |
| AI that advises, never commands | No code path exists in which an AI output writes a parameter or issues a vehicle command without an explicit, logged operator action. |
| Traceability from day one | CI fails when flight-influencing code lacks `@req:` coverage. The RTM is generated, never hand-maintained. |
| Depth over breadth | A phase does not open until the previous phase's validation gate (ROADMAP.md) is green. Feature requests that skip gates are refused. |

## Who we serve, in priority order

1. **UAV startups and OEMs** — need a complete ops stack without an integration team.
2. **Robotics companies on ROS 2** — need MAVLink and ROS 2 to coexist without custom glue.
3. **Research labs** — need reproducible simulation and log analysis.
4. **Enterprise operators (inspection, mapping)** — need fleet visibility and compliance records.
5. **Defense/government (future)** — need air-gap, auditability, and supply-chain honesty.
6. **Training institutes (future)** — need scenario-based scoring on the same platform pilots will use operationally.

Priority order resolves design conflicts: when a feature helps segment 5 but hurts segment 1's onboarding, segment 1 wins until Phase 4.

## Guiding principles (from the constitution, made operational)

1. **Build above open ecosystems** — our value is composition, so upstream health is our health; we contribute fixes upstream rather than patching locally.
2. **Prefer adapters over forks** — see test above.
3. **Architecture before implementation** — no service is coded before its section in MICROSERVICES.md and its `.proto` contract exist.
4. **Documentation is part of the product** — a doc that disagrees with the code is a P1 defect.
5. **Validate continuously** — five-layer validation (requirements, architecture, code, integration, compliance) every phase; see VALIDATION.md.
6. **Reliability over feature count** — the hardest problem is operational reliability *between* layers; we staff and schedule for that, not for tab count.

## What we say no to (current phase)

- Cloud dependencies of any kind before Phase 4.
- New vehicle protocols (DJI, proprietary FPV links) before OQ-1 is resolved by ADR — the directories exist, the scope decision does not.
- Custom map renderers, custom message buses, custom physics — composition over invention.
- Certification *claims*. We claim certification *readiness* and can show the artifacts.
