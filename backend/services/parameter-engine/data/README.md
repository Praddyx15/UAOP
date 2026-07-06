# Parameter metadata seed data

**Source:** [FlightMD](https://github.com/Praddyx15/FlightMD) (`flightmd_core/data/`), MIT License, © Pradum Behl. Imported 2026-07-07 per [DECISIONS.md](../../../../docs/00_Project_Foundation/DECISIONS.md) Review R4 / ADR-0018.

## Contents

- `px4_param_defaults/v1_13.json`, `v1_14.json` — PX4 firmware default parameter values per version, used by FlightMD's own parameter-anomaly analyser to diff a flight's actual parameter set against firmware defaults.
- `param_safe_ranges.json` — safe operating ranges and dangerous-combination rules for tuning-sensitive parameters (e.g. `BAT_LOW_THR` vs `BAT_CRIT_THR` ordering).

## How parameter-engine uses this

Directly seeds [PRODUCT_REQUIREMENTS.md](../../../../docs/00_Project_Foundation/PRODUCT_REQUIREMENTS.md) UAOP-HLR-020/021 (parameter read/validate/write) and [IMPLEMENTATION_PLAN.md](../../../../docs/07_Implementation_Guides/IMPLEMENTATION_PLAN.md) task M4.4 — parameter-engine's metadata packs are extensions of this seed data (additional PX4/ArduPilot firmware versions, additional safe-range rules) rather than authored from scratch. TUNING_ENGINE.md's risk-classified write path (LOW/MEDIUM/HIGH) reads range/combination metadata from this same source family.

## Scope note

This is the same author's own prior work (Pradum Behl, Sixty Motion Aerospace), reused under its MIT license — no third-party licensing question. Firmware coverage is currently PX4 v1.13/v1.14 only; ArduPilot 4.5 metadata (also referenced in M4.4) is not yet seeded here and remains to be authored or sourced when that task starts.
