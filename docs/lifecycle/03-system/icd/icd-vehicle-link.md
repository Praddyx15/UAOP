# ICD: vehicle link (MAVLink)

Issue 1, 2026-11-21. Between the planned `vehicle-link` item and the
autopilot. **Planned: no code implements this interface.** The design is in
`docs/02_System_Architecture/MAVLINK_INTEGRATION.md`; this file records only
what the lifecycle set relies on.

| Aspect | As designed |
|---|---|
| Protocol | MAVLink v2; PX4 and ArduPilot dialect differences handled in adapters |
| Transport | Serial, UDP, TCP. The local simulator profile exposes UDP port 14550 (`README.md`) |
| Telemetry in | Heartbeat, system status, GPS, position, attitude and the further messages listed in the design document, section 3 |
| Commands out | MAVLink command protocol with acknowledgement tracking and retry; mission and parameter protocols with read-back |
| Link loss | No heartbeat for 3 s gives the link-lost state (SYS-003) |
| Signing | MAVLink 2 message signing per link; required on wide-area links by policy |
| Observer mode | Receive only; no frames sent (SYS-016) |

Safety notes: this interface is where a wrong or repeated command reaches
the aircraft (FC-1.2, FC-1.3) and where stale data enters (FC-2.2). Its
requirements and low-level design are to be written before its code
(gap analysis G-8).

Differences found: the design documents say the bridge directory was
created; it is not in the tree (PR-009).
