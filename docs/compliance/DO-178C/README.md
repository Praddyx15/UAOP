# DO-178C compliance package

Process-assurance artifacts (COMPLIANCE.md §A — "how UAOP is built", distinct from the product's regulatory *feature* set in COMPLIANCE.md §B). Organized by DO-178C Annex A table, per ADR-0010's DO-178C↔DO-278A alignment posture: apply the process objectives, never claim certification, maintain certification-*readiness* evidence honestly — including honestly showing what doesn't exist yet.

| Table | Scope | |
|---|---|---|
| [A-2 — Planning](A-2_Planning/README.md) | PSAC, SDP, SVP, SCMP, SQAP, standards, tool qualification | Define the lifecycle before building |
| [A-3 — Development](A-3_Development/README.md) | HLR, LLR, architecture/design, source code | Build it, with traceability |
| [A-4 — Verification](A-4_Verification/README.md) | Test cases/procedures, test results, review records, RTM | Verify it — proof, not debugging |
| [A-5 — Configuration Management](A-5_Configuration_Management/README.md) | SCI, baselines, change logs | Control it — reproducibility |
| [A-6 — Quality Assurance](A-6_Quality_Assurance/README.md) | SQA reports, audit logs, non-conformance reports | Audit the process itself |
| [A-7 — Verification of Outputs](A-7_Verification_of_Outputs/README.md) | MC/DC coverage, structural coverage, end-to-end RTM, problem reports | Prove every logic path was verified |

[`RTM.md`](RTM.md) — the generated Requirements Traceability Matrix (`compliance/tools/rtm-generator/rtm_gen.py`) — lives at this directory's root rather than inside a table folder because both A-4 and A-7 reference the same artifact; it stays a single source of truth instead of being duplicated or symlinked.

**What this package is not:** a claim of certification, or a claim that every row is populated. Several A-2/A-5/A-6/A-7 artifacts are honestly marked "not yet authored" — see each table's README for the specific gap and what (if anything) substitutes for it today.
