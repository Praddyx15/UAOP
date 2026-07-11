# A-5 — Configuration Management

DO-178C Annex A Table A-5 artifacts: version control and reproducibility — "if you can't reproduce it, you can't certify it."

| Artifact | Where it lives | Notes |
|---|---|---|
| SCI (Software Configuration Index) | Not yet authored as a standalone document | Substance today: `git log`/tags are the authoritative build-identity record; a formal SCI (enumerating exact artifact versions per release) is Phase 4 scope (COMPLIANCE.md §B.2, SAS) |
| Baselines | Git tags (none cut yet — Phase 1 has no release milestone before M-series completion) | |
| Change Logs | Git commit history + `docs/context/phase-1/progress.md` (session-by-session engineering log) | `progress.md` is more granular than a typical DO-178C change log — it records *why*, not just *what changed* |

Configuration management substance = git discipline (REPOSITORY_STRUCTURE.md) + signed releases (planned, not yet in effect — no release has been cut) + the SCMP (A-2, itself still pending consolidation).
