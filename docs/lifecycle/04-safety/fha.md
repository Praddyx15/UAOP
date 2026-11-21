# Functional hazard assessment

UAOP-FHA, issue 1, 2026-11-21. An assessment in the manner of SAE ARP4761A,
sized for this product: the functions, how each can fail (loss, erroneous or
misleading, untimely), the effect, the severity and the assurance level
proposed for the items that implement it. Clause references are left out
until the editions are held (to confirm). **Nothing here is agreed:
severities and levels are proposals to be agreed with the authority.** Not
yet reviewed by a second person.

Inputs: `docs/lifecycle/03-system/conops.md`, `system-architecture.md`, the
code and design documents as read. Outputs: the failure conditions below,
`safety-requirements.sdoc` (SAF-001 to SAF-013) and `fmea.md`. Security is
treated in `docs/06_Compliance_and_Quality/SECURITY.md`; this assessment
links to it where a failure condition can be caused on purpose.

## 1. Severity classes and levels

Effects are judged on the unmanned aircraft and its operation: people and
property on the ground, other airspace users, and the pilot's ability to
control the vehicle. Effects on evidence are judged by the decisions a wrong
record could later mislead.

| Class | Meaning here | DO-278A level proposed (mapping to confirm) |
|---|---|---|
| Catastrophic | Could cause loss of life | AL1 |
| Hazardous | Large reduction in safety margins; the pilot can no longer be relied on to keep the vehicle safe; risk to people where the operation is near them | AL2 |
| Major | Significant reduction in margins or in pilot capability, for example loss of the command link with the vehicle on its lost-link action | AL3 |
| Minor | Slight reduction in margins, or a latent effect such as a misleading record | AL5 |
| No safety effect | Nothing beyond the loss of the function itself | AL6 |

AL4 is not proposed for anything here. Severity for an unmanned aircraft
depends on the operation (mass, energy, where it flies); the classes below
take a small aircraft that may fly near people or beyond visual line of
sight, and are to be redone per operation with the operator's risk
assessment (JARUS SORA).

Assumptions (each changes the result if false): A1 to A4 of the ConOps,
section 6. In short: the autopilot has a lost-link action; the pilot has
independent control where the operation needs it; the operator owns the
operational risk assessment; no real vehicle is connected today.

## 2. Functions

| # | Function | Items | Built? |
|---|---|---|---|
| F1 | Command the vehicle (commands, missions, parameters) | `gcs`, `gateway`, `vehicle-manager`, `mission-engine`, `parameter-engine`, `vehicle-link` | Gateway forwarding only |
| F2 | Display vehicle state | `vehicle-link`, `bus`, `gateway`, `gcs` | Relay only; station shows generated data |
| F3 | Monitor the vehicle link | `vehicle-link`, `gcs` | Template logic only |
| F4 | Record commands and changes (audit) | `vehicle-manager`, `compliance-engine`, `bus`, `storage` | Schema only |
| F5 | Capture flights, analyse, produce evidence | `flight-log`, `compliance-engine`, `gcs` | Analysis only |
| F6 | Run the platform (authentication, bus, stores) | `gateway`, `bus`, `storage`, `common` | Partly |

## 3. Failure conditions

| Id | Fn | Failure condition | Effect | Class | Mitigation, existing or planned | Safety req. |
|---|---|---|---|---|---|---|
| FC-1.1 | F1 | Loss of the ability to command | The pilot cannot command through UAOP; the vehicle continues its mission or its lost-link action | Major | Autopilot failsafe (A1); independent control (A2) | SAF-011 |
| FC-1.2 | F1 | Erroneous command: wrong vehicle, wrong kind or wrong parameters (for example disarm in flight, wrong mode) | Loss of control or an unintended manoeuvre | Hazardous | Closed command set and unknown commands refused (built at the gateway); autopilot's own checks; typed parameters (planned) | SAF-005 |
| FC-1.3 | F1 | A command executed more than once after a retry or redelivery | A repeated action the pilot did not intend | Major | Idempotency key and deduplication (contract and library built; not enforced) | SAF-006 |
| FC-1.4 | F1 | A command late, or its outcome shown wrongly (unknown shown as done or as failed) | The pilot acts on a false belief about vehicle state | Major | Asynchronous resolution with a distinct timed-out state (contract built) | SAF-007 |
| FC-1.5 | F1 | A command from a person or session without authority | As FC-1.2, caused on purpose or by mistake | Hazardous | Token check (built for REST); role and controlling authority (planned) | SAF-008 |
| FC-1.6 | F1 | A command sent by software with no person asking | As FC-1.2 | Hazardous | Analysis has no path to command subjects (holds today) | SAF-009 |
| FC-1.7 | F1 | A wrong mission, geofence or parameter loaded, or a partial upload taken as complete | Flight outside the intended path or with unsafe tuning | Hazardous | Safe-range data (exists), read-back and verify (planned) | SAF-010 |
| FC-2.1 | F2 | Loss of the state display, evident to the pilot | Loss of awareness; pilot reverts to other means or lets the mission run | Major beyond visual line of sight; Minor within | Autopilot continues; staleness shown (planned) | SAF-001 |
| FC-2.2 | F2 | Misleading state: old or frozen data shown as current, values of another vehicle, wrong units | The pilot makes a wrong decision or misses a developing problem | Hazardous beyond visual line of sight; Major within | Sequence, gap and heartbeat frames (built); source time and staleness marking (planned) | SAF-001, SAF-003 |
| FC-2.3 | F2 | Simulated or generated data shown as a real vehicle's | As FC-2.2 | Hazardous if a real vehicle is connected | Provenance flag in the envelope (built); marking on the display (not built; PR-001) | SAF-002 |
| FC-3.1 | F3 | Link loss not detected, or detected late | The pilot believes the link is up; as FC-2.2 | Major | Heartbeat timeout at 3 s (planned) | SAF-004 |
| FC-3.2 | F3 | Link shown as lost when it is not | Unneeded abort or distraction | Minor | Timeout chosen above normal jitter; see PR-012 | - |
| FC-4.1 | F4 | An audit record lost, or changed without detection | A later investigation is misled | Minor | Hash chain and database rules (schema built); verifier (planned) | SAF-012 |
| FC-4.2 | F4 | A failure of the record path blocks a command, including an emergency one | The pilot cannot send return or land | Major | Local log for emergency commands (planned, ADR-0015) | SAF-012 |
| FC-5.1 | F5 | An evidence bundle that verifies but is wrong or incomplete | Wrong conclusion after the flight | Minor | Content hashes; offline verifier (planned) | - |
| FC-5.2 | F5 | The system states that the aircraft is ready or may fly when it should not | A flight starts with a fault or without authorisation | Major | Rule that the system reports evidence only; not met by the station as built (PR-002) | SAF-013 |
| FC-5.3 | F5 | A wrong analysis recommendation is applied | Unsafe tuning on the next flight | Major | Advisory only, human decision, safe-range check (planned) | SAF-009, SAF-010 |
| FC-6.1 | F6 | Telemetry or events disclosed to a party without access | No effect on the flight; confidentiality | No safety effect | Authentication on streams (not built; PR-004) | - |
| FC-6.2 | F6 | Loss of the bus or a store | Becomes FC-1.1, FC-2.1 or FC-4.2 | Major | Service restart; health reporting (built in the library) | SAF-011, SAF-012 |

## 4. Proposed assurance levels

| Items | Drives it | Proposed level | Rationale |
|---|---|---|---|
| `vehicle-link`, `vehicle-manager`, gateway command route, `mission-engine` and `parameter-engine` write paths | FC-1.2, 1.5, 1.6, 1.7 | AL2, or AL3 where the authority accepts credit for the autopilot's checks and for independent pilot control | A wrong command can remove control of the aircraft; severity depends on the operation |
| `vehicle-link` receive side, gateway stream relay, station state display, link monitoring | FC-2.2, 2.3, 3.1 | AL2 for use beyond visual line of sight; AL3 otherwise | The display may be the pilot's only awareness |
| `compliance-engine`, audit store, `flight-log` capture | FC-4.1, 5.1 | AL5 | Latent, evidential effect |
| Emergency-command path through the record store | FC-4.2 | Same level as the command path | It is part of that path |
| Analysis sidecar, map, panels, `tools` | FC-5.3 with its mitigations | AL5 or AL6 | Advisory; a person decides |
| Station text that could read as a verdict | FC-5.2 | AL3 until removed | Simplest mitigation is not to make the statement |

Observer mode: with no command authority F1 does not exist and the highest
condition is FC-2.2, for a display that is not the controlling station:
Minor to Major depending on whether the pilot relies on it.

The repository's documents name a "DAL C-equivalent" tier for the command
modules. This assessment proposes a higher level for some uses; the
difference is PR-013 and is for the owner and the authority to settle.

## 5. Not done

No fault tree, no common cause analysis (bus and database are shared by
the command and record paths), no quantitative targets, no human-factors
assessment of the display. These are planned after the vehicle link exists.
