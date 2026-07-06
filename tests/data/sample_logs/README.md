# Sample flight log fixtures

**Source:** [FlightMD](https://github.com/Praddyx15/FlightMD) (`tests/sample_logs/` + `scripts/generate_sample_logs.py`), MIT License, © Pradum Behl. Imported 2026-07-07 per [DECISIONS.md](../../../docs/00_Project_Foundation/DECISIONS.md) Review R4 / ADR-0018.

## What these are

Six small, spec-valid **synthetic** binary flight logs — real binary framing (not mocks) for all three formats UAOP's ecosystem touches: PX4 ULog (`.ulg`), ArduPilot dataflash (`.bin`), and MAVLink telemetry (`.tlog`). Each format has a **clean/flawed** pair:

- **clean** — near-zero oscillation, healthy battery, stable GPS.
- **flawed** — sustained ~1.1 Hz roll/pitch oscillation, high battery sag, a mid-flight satellite drop. Chosen to mirror a real tuning issue found during FlightMD's own manual testing, so these fixtures exercise the same finding types real drones actually trigger.

`generate_sample_logs.py` is the generator — it exercises each format's actual parsing library (`pyulog` / `DFReader` / `pymavlink`) end-to-end rather than only internal field-mapping logic, and can produce more variety if UAOP needs additional synthetic profiles later.

## How UAOP uses them (current and planned)

- **`sample_clean.tlog` / `sample_flawed.tlog`** — replay-able MAVLink message streams; usable now as a seed corpus for `mavlink-bridge`'s parser hardening and fuzz harness (IMPLEMENTATION_PLAN.md task M2.3).
- **`sample_clean.ulg` / `sample_flawed.ulg` / `.bin` pairs** — seed data for `flight-log-engine` (M5.2) and, in Phase 2, for exercising the `ai-engine` → `flightmd_core` integration path end-to-end without needing a real flight.

## Scope note

This is a small, purpose-built fixture set (~1.5 MB total), not the full real-world validation corpus. A separate, much larger set of real PX4 logs (50 flights across 11 vehicle types, ~8.5 GB, sourced from PX4's public Flight Review database) exists at the FlightMD project source and was **deliberately not imported** here — bringing it in requires Git LFS setup first (tracked as a bench-task candidate in IMPLEMENTATION_PLAN.md §2, not yet scheduled).
