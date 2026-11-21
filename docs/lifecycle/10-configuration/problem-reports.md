# Problem reports

Register started 2026-11-21. Every report below was **found by reading the
code and documents, not reproduced** by running anything, and **none is
fixed**: this phase changes no existing file. A report is closed by a change
that names it. Reports are never deleted.

Kinds: D defect in code; M mismatch between documents and code or between
documents; P process. Safety column: the failure condition of
`docs/lifecycle/04-safety/fha.md` it bears on.

| Id | Kind | Where | Report | Safety | State |
|---|---|---|---|---|---|
| PR-001 | D | `frontend/qt-desktop-gcs/src/TelemetryController.cpp`; `src/main.cpp` | The station's vehicle state (position, attitude, speed, battery, satellites, link and jamming indicators) is produced by a local timer and random numbers, starting from fixed coordinates, and is shown in the live views with no "simulated" mark. The station has no connection to the gateway stream | FC-2.3 | Open |
| PR-002 | D | Same file, `readyToFly()`, `statusMessage()`; `ui/Shell.qml`; `ui/views/compliance/ComplianceView.qml` | The station shows "READY TO FLY" and "NO-GO" texts, and the compliance view shows items such as a verified and signed envelope, a fixed hash text marked as validated and sensor checks as PASS, all driven by the generated ready flag. `README.md` and the design say the product reports evidence and never says go or no-go, and never shows a compliance state it cannot prove | FC-5.2 | Open |
| PR-003 | D | `TelemetryController::armVehicle`, `disarmVehicle`, `abortMission`; `ComplianceView.qml`, `MissionView.qml` | Arm, disarm and abort change local variables only (abort also sets displayed height and speed to zero). No request goes to the gateway and no audit event is made, while the screen shows the result as if a command had been carried out | FC-1.4 | Open |
| PR-004 | D | `backend/api-gateway/src/streams/websocket.js`; `src/index.js` | The WebSocket accepts any connection without a token (the token check is an HTTP handler and does not apply to it), and builds bus subjects from the client's `vehicle`, `category` and `domain` strings unchecked, so wildcard characters or a missing field reach the subject | FC-2.2, FC-6.1 | Open |
| PR-005 | D | `backend/api-gateway/src/routes/commands.js`; `auth/middleware.js` | No role check on the command routes: `requireRole` is defined and never used, so a token with the OBSERVER role can submit commands. The caller's identity is not passed to the service | FC-1.5 | Open |
| PR-006 | D | `backend/api-gateway/src/streams/websocket.js`, `pump()` | The frame time `ts` is the gateway's send time, not the time of the data, so delayed data arrives with a fresh time. The gap frame is sent only together with the next relayed message, so drops are not reported while nothing else is relayed | FC-2.2 | Open |
| PR-007 | D | `backend/api-gateway/src/routes/auth.js`; `src/config.js` | The login route issues a token for any role to anyone, with no credentials, and the signing secret falls back to a fixed value when the environment variable is unset. Both are commented as development-only, but nothing prevents starting the gateway this way outside development | FC-1.5 | Open |
| PR-008 | M | `docs/03_Software_Architecture/API_SPECIFICATION.md`; gateway `index.js`, `config.js` | The specification gives REST under `/api/v1` and the WebSocket on port 8081. The gateway serves routes at the root and the WebSocket on the HTTP port; the `wsPort` setting is read and never used | - | Open |
| PR-009 | M | `docs/00_Project_Foundation/MASTER_CONTEXT.md` section 12 | The open question on the MAVLink bridge is marked resolved with the words that `middleware/mavlink-bridge/` was created. No `middleware/` directory is in the tree | - | Open |
| PR-010 | D | `backend/api-gateway/src/routes/commands.js` | A command without an `Idempotency-Key` header is forwarded with an empty key, and the vehicle identifier and `params` are forwarded without any check. The call has no deadline, so a hung service leaves the client waiting | FC-1.2, FC-1.3 | Open |
| PR-011 | P | `docs/compliance/DO-178C/RTM.md`; `compliance/tools/rtm-generator/rtm_gen.py` | Only one requirement id (`UAOP-NFR-008`) is annotated in code. Five of its seven test ids cover the logger, configuration, metrics, health and bus adapter, which that requirement (no exceptions or allocation on real-time paths) does not describe. The statement in `docs/06_Compliance_and_Quality/COMPLIANCE.md` that CI fails when flight-influencing code lacks annotations was not seen in the workflow (to confirm) | - | Open |
| PR-012 | D | `tools/service-template/src/domain/link_monitor.h`, `on_tick()` | `now_ms - last_seen_ms_` is unsigned. A tick time earlier than the last heartbeat time wraps to a very large value and reports the link as lost. `on_heartbeat()` rejects time going backwards; `on_tick()` does not | FC-3.2 | Open |
| PR-013 | M | `docs/00_Project_Foundation/MASTER_CONTEXT.md` section 8; `docs/06_Compliance_and_Quality/COMPLIANCE.md` section A; `docs/compliance/DO-178C/` | The documents apply DO-178C to ground software with DO-278A as a note, use "DAL C-equivalent" and "DAL D-equivalent" as level names, and cite ARP4754A and ARP4761. This lifecycle set proposes DO-278A as the frame, levels AL2 to AL6, and the current editions (to confirm). The mapping file promised at `docs/compliance/DO-178C/objective-mapping.md` does not exist | All | Open |
| PR-014 | M | `docs/00_Project_Foundation/MASTER_CONTEXT.md` section 4; `CMakeLists.txt` | The technology table gives C++17 for the core services; the build sets C++20 for everything (the decision on C++20 is recorded for the station; whether it covers the services is to confirm) | - | Open |
| PR-015 | D | `backend/api-gateway/src/index.js`; `routes/auth.js` | The logout route is installed before the handler that identifies the caller, and it requires the caller's identity. As wired it always answers 401, so a token cannot be revoked through the gateway | FC-1.5 | Open |

Order of work proposed: PR-001 to PR-006 and PR-015 before any real
vehicle is connected; PR-013 before any document is shown to an authority.
