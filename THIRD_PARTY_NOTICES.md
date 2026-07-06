# Third-party notices

Provenance register for code and data ported into `uaop-platform` under **ADR-0019** (license-tiered upstream reuse — see `docs/00_Project_Foundation/DECISIONS.md`). Every ported file carries a provenance header naming its source; every source appears here. The CI license gate (`tools/ci/check_licenses.py`) enforces both directions.

| Source | License | What was taken | Where it lives |
|---|---|---|---|
| [github.com/Praddyx15/FlightMD](https://github.com/Praddyx15/FlightMD) | MIT © Pradum Behl | Sample flight-log test fixtures (6 files, 3 formats) + generator script; PX4 v1.13/v1.14 parameter defaults + safe-range JSON | `tests/data/sample_logs/` · `backend/services/parameter-engine/data/` |

Planned (per ADR-0019, entries added at port time): PX4-Autopilot parameter/airframe metadata (BSD-3-Clause); QGroundControl MissionManager/ParameterManager state-machine logic under its Apache-2.0 option (per-file verified).

**Never eligible:** ArduPilot, Mission Planner (GPL-3.0 — Tier 3, behavioral reference only).
