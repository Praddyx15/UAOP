# Concept of operations

UAOP-CONOPS, issue 1, 2026-11-21. Short on purpose: the vision, personas and
roadmap are in `docs/00_Project_Foundation/` (`VISION.md`, `MISSION.md`,
`PRODUCT_REQUIREMENTS.md` section 1, `ROADMAP.md`) and are not repeated.
Not yet validated with operators.

## 1. The system and its boundary

The system of interest is an unmanned aircraft system: vehicle, autopilot
(PX4 or ArduPilot), radio links, control station and crew. UAOP is software
of the control station. It sits above the autopilot and never replaces it.

Outside UAOP: the autopilot and its failsafes, the radios, the remote
control transmitter, any other control station flying the same vehicle, and
the operator's procedures and authorisations.

## 2. Users

| User | Does |
|---|---|
| Remote pilot | Watches vehicle state, sends commands (planned), acts on link loss |
| Integration engineer | Reads and writes parameters (planned), reads post-flight reports |
| Safety or compliance officer | Reads the audit record and the evidence bundle; verifies them offline |
| Assessor or authority | Reviews how the software was built and what it claims |

## 3. Modes of use

| Mode | UAOP's authority | State |
|---|---|---|
| Observer | None. Receives a forwarded MAVLink stream or imported logs while another station flies the vehicle. Records and reports only | Planned (M2E) |
| Controlling station | Sends commands, missions and parameters to the autopilot; exactly one controlling session per vehicle | Planned (M3 onwards) |
| Post-flight | None. Analyses a captured log and shows the report | Partly built (analysis sidecar and station panel) |
| Simulation | Commands a simulated vehicle (PX4 simulator in Docker) | Planned (M2); data must stay marked as simulated |

## 4. Operating scenarios

1. **Normal flight, controlling.** The pilot sees position, attitude,
   battery and link state updated at least ten times a second. A command is
   sent once, recorded before it is acted on, and resolves to accepted,
   rejected or unknown. The pilot is never shown an outcome the system does
   not know.
2. **Link loss.** Within seconds the station shows the link as lost and the
   last state as old. The autopilot runs its own lost-link action; UAOP does
   not need to be alive for that.
3. **Station or service failure.** A failed service restarts; the display
   shows which data is old. The vehicle is unaffected.
4. **Observer.** UAOP beside another station: it can show and record, and
   cannot command.
5. **After the flight.** The log, parameters, analysis report and audit
   extract are bound into a bundle that a third party checks offline. The
   bundle reports evidence; it does not say the aircraft may fly.

## 5. Environment and constraints

- No internet is assumed at any time.
- Workstation or edge computer, x86_64 or ARM64, Linux; the station also
  targets desktop systems supported by Qt 6.
- One contributor today; no organisation, no approval, no flight use.

## 6. Assumptions the safety assessment depends on

- A1. The autopilot has a lost-link action configured for the operation.
- A2. The pilot has a means of control independent of UAOP where the
  operation requires one (for example the remote control transmitter).
- A3. The operator holds the operational authorisation and its risk
  assessment; UAOP supplies evidence to it and decides nothing.
- A4. Until the vehicle link exists, UAOP is not connected to a real
  vehicle.
