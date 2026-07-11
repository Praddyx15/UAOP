# A-2 — Planning

DO-178C Annex A Table A-2 artifacts: the plans that define the software lifecycle, DAL strategy, and compliance approach before development starts. Per COMPLIANCE.md §A.3, these are produced as real documents during Phase 1 (not deferred to Phase 4) — "certification-ready, not certified" means the plans exist and are followed, even though no authority has reviewed them yet.

| Artifact | Status | Notes |
|---|---|---|
| PSAC (Plan for Software Aspects of Certification) | Not yet authored | Scopes DAL C-equivalent vs DAL D-equivalent modules per COMPLIANCE.md §A.2; names DO-278A as the applicable ground-software analog (ADR-0010) |
| SDP (Software Development Plan) | Partially satisfied | CODING_STANDARDS.md, REPOSITORY_STRUCTURE.md, and this doc set's numbered-area structure cover the substance; a formal SDP document consolidating them is still pending |
| SVP (Software Verification Plan) | Partially satisfied | TESTING.md + VALIDATION.md cover the substance; formal SVP pending |
| SCMP (Software Configuration Management Plan) | Partially satisfied | Git discipline (REPOSITORY_STRUCTURE.md, this project's commit/branch conventions) + CI_CD.md cover the substance; formal SCMP pending |
| SQAP (Software Quality Assurance Plan) | Not yet authored | The phase-gate checklist discipline in VALIDATION.md is the working substitute today |
| Software Standards (coding, design, requirements) | Satisfied | CODING_STANDARDS.md, `.clang-format`/`.clang-tidy`, `compliance/tools/misra-config/` |
| Tool Qualification Plan | Not yet authored | DO-330 applies the moment `compliance/tools/rtm-generator/` or a future certification test runner's output enters certification data (COMPLIANCE.md §A.1) |

**Honest posture:** several of these plans exist today only as *substance distributed across other docs*, not as consolidated, DO-178C-labeled documents. Consolidating them is real work, tracked against IMPLEMENTATION_PLAN.md — this README exists so the gap is visible and mechanically checkable, not silently assumed complete.
