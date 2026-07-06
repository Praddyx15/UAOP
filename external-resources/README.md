# external-resources

Reference material only — **UAV/robotics ecosystem repositories studied for integration design** (e.g., px4_sim_ros2, PX4 swarm/avoidance examples, ArduPilot simulation stacks, ros_gz templates — see the GitHub Ecosystem list in `docs/00_Project_Foundation/MASTER_CONTEXT.md`).

Rules (REPOSITORY_STRUCTURE.md rule 5; enforced by `.gitignore` — only this README is tracked):

1. Never imported, linked, built, or packaged. Study material, not dependencies — UAOP consumes ecosystems via adapters (`middleware/`, `integrations/`), never by vendoring.
2. Excluded from licensing scans and release artifacts; their licenses are not our licenses.
3. Anything needed at build time graduates to a pinned, declared dependency through an ADR — it does not live here.
4. Non-UAV material does not belong here (previous unrelated content was removed 2026-07-04, review R2/F15).
5. **License tiers govern what may leave this directory (ADR-0019).** Clones of PX4-Autopilot / QGroundControl studied here may have code *ported* into the tree under that ADR's provenance rules (Tier 1/2). Clones of **ArduPilot / Mission Planner (GPLv3, Tier 3) are study-only forever**: their code never enters `uaop-platform` in any form — including AI-"refined" versions — and is never pasted into a code-generation prompt. Behavioral facts learned from them are documented in our own words (quirk tables, MAVLINK_INTEGRATION.md §2) and implemented fresh.
