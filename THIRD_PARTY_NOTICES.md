# Third-party notices

Provenance register for code, data, and assets embedded in `uaop-platform` under **ADR-0019** (license-tiered upstream reuse for code — see `docs/00_Project_Foundation/DECISIONS.md`; the same provenance discipline is applied here to bundled fonts, a non-code asset class ADR-0019 didn't originally anticipate but which follows the identical rule: permissive license, documented source, no third-party licensing ambiguity). Every ported/bundled file carries a provenance header or adjacent license file naming its source; every source appears here. The CI license gate (`tools/ci/check_licenses.py`) enforces both directions for code.

| Source | License | What was taken | Where it lives |
|---|---|---|---|
| [github.com/Praddyx15/FlightMD](https://github.com/Praddyx15/FlightMD) | MIT © Pradum Behl | Sample flight-log test fixtures (6 files, 3 formats) + generator script; PX4 v1.13/v1.14 parameter defaults + safe-range JSON | `tests/data/sample_logs/` · `backend/services/parameter-engine/data/` |
| [github.com/itfoundry/Poppins](https://github.com/itfoundry/Poppins) (via [google/fonts](https://github.com/google/fonts/tree/main/ofl/poppins)) | SIL Open Font License 1.1 © 2020 The Poppins Project Authors | 8 static font faces (Light/Regular/Medium/Bold × upright/italic), embedded as Qt resources | `frontend/qt-desktop-gcs/resources/fonts/Poppins/` (license text: `OFL.txt` in the same directory) |

Planned (per ADR-0019, entries added at port time): PX4-Autopilot parameter/airframe metadata (BSD-3-Clause); QGroundControl MissionManager/ParameterManager state-machine logic under its Apache-2.0 option (per-file verified).

**Never eligible:** ArduPilot, Mission Planner (GPL-3.0 — Tier 3, behavioral reference only).
