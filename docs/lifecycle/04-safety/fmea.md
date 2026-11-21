# FMEA: command path and telemetry path

UAOP-FMEA, issue 1, 2026-11-21. Qualitative, by stage of the two critical
paths of `docs/lifecycle/03-system/system-architecture.md`, section 4.
Stages marked (planned) have no code; their rows state what the design must
handle. Rows for built stages are from reading the code, not from test.

## 1. Command path

| # | Stage | Failure mode | Local effect | End effect | Detection or mitigation | Ref. |
|---|---|---|---|---|---|---|
| C1 | Station action | Action changes only local state and shows a result | No command leaves the station; the display shows "armed" | Pilot believes a command was sent | None today (PR-003) | FC-1.4 |
| C2 | Station to gateway | Request lost or repeated by retry | Missing or duplicate submission | Late or repeated command | Idempotency key (forwarded; not enforced) | FC-1.3 |
| C3 | Gateway: map command | Unknown or missing command name | Refused with 400 before any call | None | Built and tested | FC-1.2 |
| C4 | Gateway: build request | Vehicle identifier or parameters malformed | Forwarded unchecked | Depends on the service refusing it | Not validated (PR-010) | FC-1.2 |
| C5 | Gateway: authorise | Token valid but role may not command; identity not passed on | Request forwarded with no role check and no actor | Unauthorised command | Not built (PR-005) | FC-1.5 |
| C6 | Gateway to service call | Service down or slow | 502 to the client; a slow call has no deadline set | Loss of command; client waits | Error returned; no timeout seen in the route | FC-1.1 |
| C7 | Vehicle manager: admit (planned) | Authority or state check wrong | Command admitted when it should not be | Erroneous command | Requirements before code (G-8) | FC-1.2, 1.5 |
| C8 | Vehicle manager: record (planned) | Audit store down | Command blocked | Emergency command lost | Local log for emergency class | FC-4.2 |
| C9 | Command subject (planned) | Redelivery | Command seen twice by the link | Repeated execution | Deduplicate by event id | FC-1.3 |
| C10 | Vehicle link: send (planned) | Wrong target system, wrong dialect mapping | Command to another vehicle or with other meaning | Erroneous command | Per-autopilot adapter tests | FC-1.2 |
| C11 | Vehicle link: acknowledgement (planned) | Acknowledgement lost | Outcome unknown | Shown as failed or done by mistake | Timed-out state, shown as unknown | FC-1.4 |
| C12 | Result to station (planned) | Result event lost | Command stays pending | Pilot unsure | Result can be polled by correlation id (route built) | FC-1.4 |

## 2. Telemetry path

| # | Stage | Failure mode | Local effect | End effect | Detection or mitigation | Ref. |
|---|---|---|---|---|---|---|
| T1 | Autopilot to link (planned) | Radio loss | No frames | Display stops updating | Heartbeat timeout 3 s | FC-3.1 |
| T2 | Vehicle link: decode (planned) | Wrong scaling or units; wrong vehicle id | Wrong values published | Misleading state | Units in names; tests against both autopilots | FC-2.2 |
| T3 | Bus | Message lost, duplicated or reordered | Gap or repeat | Old or jumping values | Per-source sequence, deduplication (library built) | FC-2.2 |
| T4 | Gateway relay: rate shaping | Messages dropped to meet the rate | Fewer frames | None if reported | Gap frame with count (built, tested); the gap frame is sent only with the next relayed frame, so a stream that stops reports nothing | FC-2.2 |
| T5 | Gateway relay: time stamp | Frame stamped with send time | Old data carries a fresh time | Stale shown as current | Not mitigated (PR-006); source time must come from the payload | FC-2.2 |
| T6 | Gateway relay: stalls | Relay or bus connection hangs | No frames | Display frozen | Heartbeat frame every 5 s tells the client the socket is alive, not that data flows; client must time out per stream | FC-2.2 |
| T7 | Gateway relay: subscription | Unvalidated subject parts; no authentication | A client receives other vehicles' data | Disclosure; values of another vehicle | Not mitigated (PR-004) | FC-2.2, 6.1 |
| T8 | Station: source of state | State generated locally | Display shows plausible moving values | Generated data taken for a vehicle's | Not mitigated (PR-001) | FC-2.3 |
| T9 | Station: display (planned) | No staleness logic | Last value stays on screen | Stale shown as current | Staleness limit per category | FC-2.2 |
| T10 | Station: status text | Fixed "ready" or "verified" text from a generated flag | Reads as a verdict | False confidence before flight | Not mitigated (PR-002) | FC-5.2 |

## 3. Findings

1. The two paths are not connected at the station: what the pilot sees and
   presses today is local to the station (C1, T8). This is acceptable only
   while no vehicle is connected (assumption A4) and should be the first
   thing closed when live telemetry arrives.
2. Staleness has three independent causes (T4, T5, T6) and needs one rule at
   the display that covers all of them (SAF-001, SAF-003).
3. Authority is checked nowhere yet (C5, C7); it must exist in the service,
   with the gateway check as a second layer.
