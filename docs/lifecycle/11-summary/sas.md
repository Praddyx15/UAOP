# Accomplishment summary (draft)

UAOP-SAS, issue 1 draft, 2026-11-21. A short, plain statement of where the
project stands against its plans (`docs/lifecycle/01-plans/plans.md`).
**UAOP is not certified, not approved and not ready to fly with. No
compliance with any standard is claimed.**

## 1. What the software is today

Foundations of a control-station and flight-evidence platform: contracts, a
C++ common library, an API gateway, a database schema, bus provisioning, a
post-flight analysis sidecar and a desktop shell. Milestones M0 and M1 are
done; M2 (first live telemetry) has not started. No vehicle link, no command
execution, no audit writer and no evidence bundle exist. The station shows
generated vehicle data.

## 2. What this lifecycle phase added

Documents only, under `docs/lifecycle/`: classification and plans; a
requirements standard and grammar; ConOps; 11 stakeholder needs; 13 safety
requirements; 30 system requirements (15 Active, 15 Proposed); 5 interface
control documents; a hazard assessment with 19 failure conditions; an FMEA
of the command and telemetry paths; 44 software high-level requirements (31
Active, 13 Proposed); 16 low-level requirements and a low-level design for
the gateway; an as-built high-level design; 15 problem reports; a gap
analysis.

## 3. Status against the plans

| Area | Status |
|---|---|
| Classification and levels | Proposed; not discussed with any authority |
| Requirements | Written after the code for built items; before the code for planned ones. Not validated with stakeholders |
| Trace | Parent links exist in the requirement files. No tool has checked them. Tests and code carry no tags for these requirements |
| Verification | Existing tests run in CI. No requirement-based test evidence is recorded; several Active requirements have no test (noted on each). Coverage is not measured |
| Reviews and independence | One contributor; no independent review or quality assurance |
| Configuration management | Git and pull requests; no release baseline or configuration index |
| Safety | Qualitative assessment only; the functions with the highest criticality are not built |

## 4. Open problem reports

Fifteen, all open (`docs/lifecycle/10-configuration/problem-reports.md`).
Those bearing on safety: PR-001 (generated state shown as live), PR-002
(ready and no-go statements), PR-003 (local actions shown as commands),
PR-004 and PR-005 (no authentication on the stream, no role check on
commands), PR-006 (frame time is not source time), PR-015 (logout cannot
revoke).

## 5. What remains

The steps of `docs/lifecycle/00-project-record/gap-analysis.md`, section 2:
tooling and tags (G-1 to G-6), the problem reports (G-7), requirements and
design for the vehicle link and vehicle manager before their code (G-8),
verification and review records (G-9), agreement with an authority and
validation with operators (G-10), and alignment of the existing compliance
documents (G-11).
