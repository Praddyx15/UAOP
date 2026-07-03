# UX_GUIDELINES

**Version 0.1.0 · 2026-07-03 · How the interface behaves under operator cognitive load. Aviation human-factors principles applied to a ground platform. Structure/content: UI_GUIDELINES.md.**

## 1. The governing scenario

Design every interaction for this user: a licensed pilot (P1), 40 minutes into a BVLOS mission, managing two aircraft, when a GPS-jamming advisory and a battery warning arrive within seconds. Interfaces that are merely fine in calm demos fail exactly here. Consumer UX optimizes for delight; ours optimizes for **correct action under stress**.

## 2. Alerting model (aviation-standard, platform law)

Three levels, one master-alert discipline:

| Level | Meaning | Behavior |
|---|---|---|
| **WARNING (red)** | Immediate operator action required (failsafe active, link lost, RID broadcast lost, geofence breach) | Master warning indicator + persistent banner + optional aural; requires acknowledgment; never auto-dismisses |
| **CAUTION (amber)** | Attention needed, action may be required (battery low tier-1, link degraded, EKF innovation high) | Master caution + alert stack entry; acknowledgeable |
| **ADVISORY (white/cyan)** | Awareness (AI findings, sync status, degradation ladder shifts) | Alert stack only; no interruption |

Rules: **no modal popups for alerts, ever** (modals steal focus from flying); alerts stack in a dedicated, always-visible region, newest-first, filterable by vehicle; acknowledgment is per-alert and logged; identical repeats coalesce with a counter (no popup storms — SYSTEM_ARCHITECTURE.md §4.3); every WARNING/CAUTION maps to a checklist/runbook link. Alert definitions are shared with the observability severity scheme (OBSERVABILITY.md §3) so the platform never disagrees with itself about what's urgent.

## 3. State honesty (the UX expression of the platform's core ethic)

- **Staleness is one visual language everywhere:** fresh (normal) → stale (age badge, desaturation at T+2 s) → lost (explicit LOST state, last-known values ghosted with timestamp). Never render stale-as-live: this is the FHA's top platform hazard (COMPLIANCE.md §A.1) surfacing as pixel policy.
- **Uncertainty is a first-class display state:** command TIMEOUT renders "state unknown — verify" (API_SPECIFICATION.md §4), not success or failure.
- **The system explains its own degradation:** ladder shifts (EDGE_ARCHITECTURE.md §4) appear as advisories in operator language — "AI analysis paused (system load); flight functions unaffected."
- **Sim/replay contexts are globally watermarked** and command-disabled (UI_GUIDELINES.md §7); mode confusion between live and not-live is treated as a top-severity design failure in review.

## 4. Workflow design

- **Golden paths are linear:** connect → pre-flight → authorise → fly → review runs left-to-right in the FLIGHT OPS workspace with the current step visually anchored. The checklist gate (UAOP-HLR-043) is a natural step, not bureaucratic friction bolted on.
- **Progressive disclosure without hiding:** dense summary always visible; detail on focus (click a battery strip → full cell/temperature/current panel docks in). Nothing operationally relevant lives *only* behind disclosure.
- **Confirmation semantics scale with consequence** (single action → confirm → typed acknowledgment, per risk classes in TUNING_ENGINE.md §4); confirmations state the *consequence*, not the question ("Write 14 parameters to veh-01 (2 HIGH-risk: MC_PITCHRATE_P, EKF2_GPS_CHECK)" — never "Are you sure?").
- **Undo where physics permits, refuse-to-guess where it doesn't:** parameter revert-to-version, mission plan versions; but no "undo ARM" theater — irreversible actions get the confirmation ladder instead.

## 5. Multi-vehicle attention management

One vehicle is always **focused** (full HUD/detail); others run as monitor strips with their alert states aggregated into per-vehicle chips. Alert in an unfocused vehicle → chip escalation + stack entry; **focus switch is one action** (click chip / hotkey 1-9) and swaps the entire workspace context atomically. The operator's mental model: "I fly one aircraft; I supervise N" — the UI enforces that instead of pretending a human can fly five.

## 6. Error prevention beats error messages

Invalid states unenterable where possible: mission validation refuses upload before hardware hears about it (MISSION_ENGINE.md §3); disabled controls always explain themselves; destructive/irreversible controls physically separated from routine ones; parameter fields enforce range/type at entry with the safe envelope drawn on sliders. When an error must be shown, the format is: what happened → what it means operationally → what to do — in operator language, error codes secondary for support reference.

## 7. Learnability & operator qualification

The instrument-panel aesthetic is deliberately familiar to pilots (EFIS conventions, aviation alert colors — STYLE_GUIDE.md); tooltips carry parameter documentation inline (the ecosystem's tribal knowledge, surfaced); a **guided SITL sandbox** (workstation profile) is the training ground — new operators learn on simulated aircraft with the identical UI, which is also the Phase 3 training system's foundation (scenario scoring bolts onto the same substrate). Target: a current QGC/Mission Planner user completes a supervised first mission within one session; full workspace fluency within a working day.

## 8. Accessibility & environment

Contrast ratios per STYLE_GUIDE.md verified for sunlight-readable field laptops (dark theme is primary; a high-brightness field variant is a Phase 3 item); color never the sole channel (alerts pair color + icon + position + text); font sizes scale per density mode without layout collapse; aural alerts optional and per-level configurable (field environments vary from silent ops to generator noise).
