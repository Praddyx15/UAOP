# UAOP: guide for coding assistants

UAOP is Pradum Behl's personal project: the flight-evidence layer for PX4 and ArduPilot, built with
AirBrief and FlightMD (ADR-0023). The founder is the sole contributor: commits and pull requests
carry no co-author or generated-by lines.

## Read before any work

1. `README.md`
2. `docs/00_Project_Foundation/MASTER_CONTEXT.md`: the constitution; it wins every conflict
3. `docs/00_Project_Foundation/DECISIONS.md`: ADRs, review log, open questions
4. The numbered doc area for the task (`docs/00_…08_`)
5. `docs/07_Implementation_Guides/IMPLEMENTATION_PLAN.md`: the current milestone (§0 and §9)
6. `docs/context/phase-1/progress.md`: the latest session entry

## Rules

Every rule in CONTRIBUTING.md applies; it is imported here.

@../CONTRIBUTING.md

Two more for assistants:

- Never paste ArduPilot or Mission Planner source into a prompt or tool (ADR-0019 Tier 3); describe
  the behaviour and implement fresh.
- Scope creep is refused, not accommodated: stay inside the current milestone.

## History

Pre-baseline planning files (idea transcripts, prototype Docker files, old assistant configs) were
removed from the tree on 2026-11-12; they remain in git history before that date under
`docs/context/legacy/`.
