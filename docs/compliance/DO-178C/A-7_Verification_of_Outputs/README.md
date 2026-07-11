# A-7 — Verification of Outputs

DO-178C Annex A Table A-7 artifacts: proof that every logic path was verified — the highest-rigor table, formally scoped to DAL A in the standard, applied here to UAOP's **DAL C-equivalent** tier (COMPLIANCE.md §A.2: mavlink-bridge, vehicle-manager, mission-engine transfer/verify, parameter-engine write path, remote-id, compliance-engine chain).

| Artifact | Where it lives | Notes |
|---|---|---|
| MC/DC Coverage Reports | Not yet produced | No DAL C-equivalent module exists yet as of M1 (mavlink-bridge lands M3+, IMPLEMENTATION_PLAN.md) — MC/DC coverage has nothing to measure until then. Tooling choice (e.g. `gcov`/`llvm-cov` + a MC/DC-capable analyzer) is an open item, not yet an ADR. |
| Structural Coverage | Not yet produced | Same dependency as above |
| RTM (End-to-End) | [`../RTM.md`](../RTM.md) | Same generated artifact as A-4 — DO-178C references it from both tables because it's the mechanism that proves both requirements-based testing (A-4) and structural-coverage completeness (A-7) |
| Problem Reports | GitHub Issues (informal) | No formal problem-report register yet, same gap as A-6's non-conformance reports |

**Honest posture:** this table is the clearest evidence that UAOP is "certification-ready, not certified" — every row here is legitimately empty today because the code it would verify doesn't exist yet. Populating it prematurely (coverage reports for code that isn't DAL C-equivalent, RTM entries invented ahead of real annotations) would be worse than leaving it visibly incomplete.
