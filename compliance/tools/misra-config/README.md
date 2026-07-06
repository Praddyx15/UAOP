# MISRA C++:2023 gate configuration

Ruleset scope and gate policy per [COMPLIANCE.md](../../../docs/06_Compliance_and_Quality/COMPLIANCE.md) §A.2 and ADR-0013/0016: **zero critical violations** on DAL C-equivalent paths (listed in `cppcheck-misra.json`), gate-blocking in CI from the first flight-influencing commit (milestone M2). Advisory-level findings are trend-reported, not blocking.

Notes:
- `rule-texts.txt` (MISRA rule descriptions) is **licensed material from MISRA and is not committed** — CI runs without it (rule IDs only, fully functional as a gate); a licensed copy can be dropped in locally for human-readable reports.
- Cppcheck's `misra.py` addon targets MISRA C; MISRA **C++**:2023 checking with full coverage requires a commercial checker (Axivion, Helix QAC, Parasoft) — a Phase 2+ procurement decision. Until then this config enforces the enforceable subset (pointer discipline, implicit conversions, unreachable code, side-effect rules) plus the project's own hard rules (no exceptions / no dynamic allocation on RT paths) via clang-tidy and compiler flags. This limitation is stated here so nobody mistakes the interim gate for full MISRA compliance — honest posture per ADR-0010.
- Suppressions require an inline justification at the code site **and** an entry in `cppcheck-misra.json` with rationale. Unjustified suppressions fail review.
