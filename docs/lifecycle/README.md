# Lifecycle data

Index of UAOP's lifecycle data: each item that DO-278A (ground software) and
SAE ARP4754B / ARP4761A (system and safety) ask for, the file that holds it,
and its state. Issue 1, 2026-11-21. This first phase is documentation only:
it was written by reading the repository as it stands (milestones M0 and M1
done, M2 not started) and it changes no code, test, configuration or
existing document.

**Nothing here claims certification, approval, compliance or an agreed
assurance level.** Levels are proposals to be agreed with the authority.
Standard clause numbers are left out, and editions are marked "to confirm",
until the copies are held.

States: **exists** (there and current), **partial** (some of it; what is
missing is said), **planned** (not started; where it will go).

## What the repository is today

UAOP is described in `README.md` as the flight-evidence layer for PX4 and
ArduPilot vehicles, with a ground control station as its wider scope
(`docs/00_Project_Foundation/MASTER_CONTEXT.md`). As found, the repository is
mostly design documents and foundations: message contracts (`api/proto/`), a
C++ common library, an API gateway (REST, WebSocket, gRPC client), database
migrations, bus provisioning, a post-flight analysis sidecar and a Qt desktop
shell whose vehicle state is still generated locally. There is no vehicle
link, no telemetry engine and no command execution yet. The lifecycle set is
sized to that: system level, safety and plans in full; software requirements
as built plus the planned ones marked Proposed; low-level requirements for
the API gateway only.

## Classification (stated in `01-plans/plans.md`, section 1)

The control station is part of the unmanned aircraft system. UAOP is ground
software, so the process frame proposed is DO-278A, with ARP4754B and
ARP4761A at system and safety level. DO-178C is airborne and is not the frame
here; the autopilot stays outside UAOP. The command and telemetry paths carry
the highest criticality. All of this is to be agreed with the authority.

## Folders

| Folder | Holds |
|---|---|
| `00-project-record/` | Gap analysis (before, after, what a later phase needs) |
| `01-plans/` | Plan for software aspects of approval, development, verification, configuration management, quality assurance (one file) |
| `02-standards/` | Requirements standard, StrictDoc grammar, pointer to the coding rules |
| `03-system/` | ConOps, stakeholder needs, system requirements, system architecture, interface control documents (`icd/`) |
| `04-safety/` | Functional hazard assessment, FMEA, safety requirements |
| `05-requirements/` | Software high-level requirements; low-level requirements for the API gateway |
| `06-design/` | High-level design as built; low-level design for the API gateway |
| `10-configuration/` | Problem reports |
| `11-summary/` | Accomplishment summary (draft) |

Folders 07 (verification), 08 (traceability) and 09 (reviews) are planned;
the numbers are kept free for them.

## DO-278A software lifecycle data

Item names follow DO-278A (to confirm against the copy held).

| Item | File | State |
|---|---|---|
| Plan for Software Aspects of Approval | `01-plans/plans.md` section 1 | Exists (not agreed) |
| Software Development Plan | `01-plans/plans.md` section 2; `CONTRIBUTING.md`; `docs/07_Implementation_Guides/IMPLEMENTATION_PLAN.md` | Exists |
| Software Verification Plan | `01-plans/plans.md` section 3; `docs/06_Compliance_and_Quality/TESTING.md`, `VALIDATION.md` | Exists |
| Software Configuration Management Plan | `01-plans/plans.md` section 4 | Exists |
| Software Quality Assurance Plan | `01-plans/plans.md` section 5 | Exists; no independent QA role yet |
| Software Requirements Standards | `02-standards/requirements-standard.md`, `02-standards/requirements.sgra` | Exists; not machine-checked |
| Software Design Standards | `docs/03_Software_Architecture/SOFTWARE_ARCHITECTURE.md`; `tools/service-template/` | Partial: no separate standard |
| Software Code Standards | `02-standards/coding-standards.md` pointing to `docs/08_Developer_Handbook/CODING_STANDARDS.md`, `.clang-format`, `.clang-tidy`, `compliance/tools/misra-config/` | Exists for C++; partial for JavaScript and Python |
| Software Requirements Data | `05-requirements/hlr.sdoc` (44 HLR, 31 Active), `05-requirements/llr-gateway.sdoc` (16 LLR); product requirements stay in `docs/00_Project_Foundation/PRODUCT_REQUIREMENTS.md` | Partial: LLR for one item; seven system requirements of items not yet built (SYS-002, 016, 019, 020, 021, 022, 025) have no HLR yet |
| Design Description | `06-design/hld.md`, `06-design/lld-gateway.md`; design intent in `docs/02_System_Architecture/` and `docs/03_Software_Architecture/` | Partial |
| Source Code | `api/`, `backend/`, `frontend/`, `infrastructure/`, `tools/` | Partial: foundations only |
| Executable Object Code | Built by CI (`.github/workflows/pr.yml`); no release yet | Planned |
| Verification Cases and Procedures | The tests beside each item; `tests/` | Partial: no requirement tags on tests |
| Verification Results | CI runs; none kept per baseline | Planned (`07-verification/`) |
| Life Cycle Environment Configuration Index | `CMakePresets.json`, `infrastructure/docker/compose/compose.yaml`, CI workflow pins | Partial: no single index |
| Software Configuration Index | - | Planned, at first release |
| Problem Reports | `10-configuration/problem-reports.md` | Exists: 15 reports, none fixed |
| Configuration Management Records | Git history; pull requests | Partial |
| Quality Assurance Records | Review log in `docs/00_Project_Foundation/DECISIONS.md` | Partial (`09-reviews/` planned) |
| Accomplishment Summary | `11-summary/sas.md` | Partial: draft |
| Trace Data | `.sdoc` parent links; `docs/compliance/DO-178C/RTM.md` (generated from code annotations) | Partial: `08-traceability/` planned |
| Tool qualification data (DO-330) | Stance in `01-plans/plans.md` section 1.5 | Planned: none needed unless credit is taken |

The existing `docs/compliance/DO-178C/` scaffold is kept as it is. Its
relation to this set is in `00-project-record/gap-analysis.md`.

## ARP4754B and ARP4761A system and safety data

| Item | File | State |
|---|---|---|
| Development and approval planning | `01-plans/plans.md` | Exists |
| Concept of operations, stakeholder needs | `03-system/conops.md`, `03-system/stakeholder-needs.sdoc` (11) | Exists; not validated with stakeholders |
| System requirements | `03-system/system-requirements.sdoc` (30, of which 15 Proposed) | Exists |
| System architecture and allocation | `03-system/system-architecture.md` | Exists |
| Interface control documents | `03-system/icd/icd-vehicle-link.md`, `icd-event-bus.md`, `icd-grpc.md`, `icd-gateway-api.md`, `icd-storage.md` | Exists; vehicle link is planned design only |
| Functional hazard assessment | `04-safety/fha.md` | Exists (not reviewed) |
| FMEA, safety requirements | `04-safety/fmea.md`, `04-safety/safety-requirements.sdoc` (13) | Partial: qualitative only |
| Fault tree, common cause analysis | - | Planned |
| System safety assessment | - | Planned, after levels are agreed |
| Security assessment | `docs/06_Compliance_and_Quality/SECURITY.md`, `SECURITY.md` | Exists as design; no threat model per item |
| Requirements validation records | - | Planned (`09-reviews/`) |

## The `.sdoc` files

The five `.sdoc` files use StrictDoc syntax and the grammar
`02-standards/requirements.sgra`. **They are not machine-checked in this
phase:** StrictDoc is not installed and no trace check runs. Syntax, unique
ids and parent links were checked by reading only. Installing the tool and
adding the check are in the gap analysis.
