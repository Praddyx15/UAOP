# Plans

UAOP-PLN, issue 1, 2026-11-21. Five short plans in one file. Each can be
split out when it outgrows a page. Nothing here is agreed with an authority.

## 1. Plan for software aspects of approval

### 1.1 System and software overview

UAOP runs on a workstation or an edge computer beside the aircraft. It is
planned to receive the vehicle's MAVLink stream, show the vehicle state,
record the flight with a tamper-evident audit of every command and parameter
change, and (from milestone M3) send commands, missions and parameters. It
never runs control loops; PX4 or ArduPilot flies the aircraft. It can also
run in observer mode beside another control station with no command
authority. The state of the build is in `06-design/hld.md`; in short, the
vehicle link and the command execution path do not exist yet.

### 1.2 Classification

| Question | Position proposed | Basis |
|---|---|---|
| What is UAOP in the system? | Part of the control station of an unmanned aircraft system (UAS). The UAS is the system of interest for safety | ARP4754B system view |
| Which software standard? | DO-278A as the process frame, because the software is on the ground. DO-178C is the airborne standard and is not the frame for this ground segment | DO-278A scope is ground CNS/ATM software; its use for a UAS control station is a proposal. An authority may instead ask for DO-178C objectives for control-station software that is part of the approved design; the existing `docs/compliance/DO-178C/` scaffold and the mapping promised in ADR-0010 cover that case |
| System and safety process | ARP4754B for development assurance, ARP4761A for safety assessment (editions to confirm) | `04-safety/fha.md` |
| Operational risk frame | JARUS SORA for the operation; the operator owns it. UAOP's part is the integrity it can offer the command and control functions | `docs/06_Compliance_and_Quality/COMPLIANCE.md` |
| Vehicle interface | MAVLink v2 (the open protocol; not an aviation standard) | `03-system/icd/icd-vehicle-link.md` |
| Control station interoperability | STANAG 4586 is noted as the reference for control-station interfaces; no conformance is planned or claimed (relevance to confirm) | - |

### 1.3 Criticality and proposed assurance levels

The functions with the highest criticality are those of the command and
control link: commanding the vehicle and displaying its state. The key
failure conditions are a misleading vehicle state and a wrong, repeated or
late command (`04-safety/fha.md`, FC-1.2, FC-1.3, FC-1.4, FC-2.2).

| Items | Proposed level | Note |
|---|---|---|
| Vehicle link, vehicle manager (command, authority), gateway command route, mission and parameter write paths | AL2 where the operation relies on UAOP to command a vehicle near people; AL3 otherwise | To be agreed with the authority; depends on credit for the autopilot's own checks |
| Telemetry path to the display, link monitoring, station display of vehicle state | AL2 for beyond-visual-line-of-sight use; AL3 otherwise | Same |
| Audit chain, flight capture, evidence bundle | AL5 | Evidence only |
| Post-flight analysis, map, panels, tools | AL5 or AL6 | Advisory |

In observer mode UAOP has no command authority and the command rows do not
apply. The repository's own documents currently name a "DAL C-equivalent"
tier for the same modules; the difference is recorded as PR-013.

### 1.4 Means proposed

Requirements at five levels with parent links (`02-standards/`), the
contracts in `api/proto/` as interface requirements, requirements-based
tests in CI on x86_64 and ARM64, static analysis for C++, reviews recorded
per pull request, and git as the configuration store. Independence of
verification is not available: the project has one contributor. That is the
largest gap against AL2 or AL3 and is stated in `11-summary/sas.md`.

### 1.5 Tools

No tool qualification is planned. No credit is taken for a tool whose output
is not also checked by review or by a second means. Candidates if credit is
ever wanted: the trace matrix generator
(`compliance/tools/rtm-generator/rtm_gen.py`) and the planned trace check.

## 2. Development plan

- Life cycle: document, contract, code, test, review, gate, as set out in
  `docs/07_Implementation_Guides/IMPLEMENTATION_PLAN.md` section 1 and
  `CONTRIBUTING.md`. Milestones M0 to M7 are in the same plan.
- Requirements: product requirements in
  `docs/00_Project_Foundation/PRODUCT_REQUIREMENTS.md`; system, safety and
  software requirements in this set, under `02-standards/requirements-standard.md`.
- Design: `docs/02_System_Architecture/`, `docs/03_Software_Architecture/`;
  as-built design in `06-design/`. New services start from
  `tools/service-template/`.
- Code: standards in `02-standards/coding-standards.md`. Languages: C++20
  (services, station), JavaScript on Node.js (gateway), Python 3.11
  (sidecar, tools), SQL, QML.
- Environment: CMake presets, Docker Compose for the local services, GitHub
  Actions for CI.

## 3. Verification plan

| Level | Method | Where |
|---|---|---|
| LLR | Unit tests beside each item | `backend/services/common/tests/`, `backend/api-gateway/tests/`, sidecar `tests/`, `frontend/qt-desktop-gcs/tests/` |
| HLR | Requirements-based tests at the item's interface; contract checks (`buf lint`, `buf breaking`) | Same, plus `tests/integration/` |
| SYS | Simulator-in-the-loop tests against PX4 and ArduPilot; fault injection; soak | Planned from M2 (`docs/06_Compliance_and_Quality/VALIDATION.md`) |
| SAF | Analysis, and tests of the SYS children | `04-safety/` |

Structural coverage is not measured today. The target proposed is statement
and decision coverage for AL3 items, with modified condition/decision
coverage added if AL2 is agreed (to confirm against DO-278A). Tests do not
yet name the requirement they verify; adding those tags is a code-touching
step and is in the gap analysis. Reviews of requirements and design are
planned in `09-reviews/`.

## 4. Configuration management plan

- Store: one git repository. `main` is the baseline branch; work lands on
  `dev` and reaches `main` by pull request. History is never rewritten on
  `main`.
- Identification: every item is a path at a commit. A release baseline is a
  signed tag with a configuration index (planned at the first release).
- Change control: one pull request per change, CI green, review recorded.
  Contracts in `api/proto/` only ever add fields. A decision that changes
  the architecture needs an ADR in `docs/00_Project_Foundation/DECISIONS.md`.
- Problem reports: `10-configuration/problem-reports.md`, one row per
  report, never deleted; a fix names the report in its commit.
- Environment: tool and image versions are pinned in `CMakePresets.json`,
  the CI workflows and `infrastructure/docker/compose/compose.yaml`;
  dependency updates arrive as their own pull requests.
- Archive and retrieval: the hosted repository and its clones. Media
  archive of baselines is planned.

## 5. Quality assurance plan

- What is checked: that each change followed sections 2 to 4 (requirement
  or problem report named, tests present, CI green, documents updated in the
  same change).
- By whom: today the single contributor, using the pull-request checklist in
  `CONTRIBUTING.md` and the review log in `DECISIONS.md`. No independent
  quality assurance exists; this is stated, not hidden.
- Records: pull requests and the review log; `09-reviews/` when created.
- Conformity review before a release: planned, with `11-summary/sas.md` as
  its output.
