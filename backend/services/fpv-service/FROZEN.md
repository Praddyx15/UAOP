# FROZEN — do not implement

This directory is **frozen pending OQ-1** (`docs/00_Project_Foundation/DECISIONS.md` §Open): FPV link support (analog vs digital — DJI O3/HDZero/Walksnail) exists in no planning document and has no ADR.

Until an ADR scopes it: no code, no contracts, no dependencies may reference this service. If adopted, it becomes a Zone 0 protocol adapter peer to mavlink-bridge (same untrusted-input discipline, same canonical telemetry contract) — see MICROSERVICES.md.

Guard: PROJECT_STRUCTURE.md §2 — CI fails on any file here other than this one and `.gitkeep`.
