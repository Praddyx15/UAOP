# Phase 1 Progress Log

Per MASTER_CONTEXT §11 and DOCUMENTATION_PROCESS §3: updated every working session — installation progress, architecture decisions, dependency issues, integration status, review findings, benchmarks, remaining work. The next session starts by reading this file.

---

## 2026-07-04 — Phase 0 wrap: workspace cleaned, structure realized

**Done:**
- Documentation baseline v0.1.x complete: 47 docs in the numbered living design system (`docs/00_…08_`), reviews R1 (12 findings, ADR-0015/0016/0017) and R2 (7 findings) in DECISIONS.md.
- Workspace cleanup (review R2): external-resources purged (302 tracked skeleton files removed; README + ignore rule remain), legacy docs/configs archived to `docs/context/legacy/`, 18.9 MB build artifacts deleted, `.gitignore` added, fresh CLAUDE.md/AGENTS.md.
- Full PROJECT_STRUCTURE directory tree created; FROZEN.md guards in dji-service/fpv-service; OQ-2 resolved (mavlink-bridge directory exists).
- Kept: Qt GCS prototype (`frontend/qt-desktop-gcs/` — CMake/C++/QML seed for M0; view-per-page layout to be refactored to dockable panels).

**M0 remaining (IMPLEMENTATION_PLAN.md §M0):**
- [ ] Root CMakePresets.json + .clang-format/.clang-tidy
- [ ] Canonical C++ service template compiling x86_64 + ARM64
- [ ] Refit GCS prototype into panel-host shell (docking framework, Theme from STYLE_GUIDE tokens)
- [ ] CI shell (.github/workflows/pr.yml: lint → build → placeholder tests → Trivy)
- [ ] setup.sh v0 (infra compose + PX4 SITL)
- [ ] OQ-3 map spike (2-day timebox) → ADR

**Blockers / founder decisions pending:** OQ-1 (dji/fpv scope — Phase 2 planning), OQ-3 spike result.

**Benchmarks:** none yet (first: 1000 Hz ingest rig lands with M2).

---

## 2026-07-06 — IMPLEMENTATION_PLAN.md expanded to a task-level WBS (v0.2.0)

**Done:**
- IMPLEMENTATION_PLAN.md rewritten with full work breakdown structure: every M0–M7 milestone broken into IDed tasks with effort estimates (focus-days), dependencies, and requirement/ADR/risk traces. Phase 1 total: **115 focus-days**, with three calendar scenarios (aggressive 5.3mo / sustainable 8.8mo / conservative 13.3mo — pick the one matching actual availability).
- Illustrative Gantt chart added (aggressive-cadence only, explicitly flagged for recalibration).
- Review R3 (5 findings, F20–F25) in DECISIONS.md: RTM-generator scaffolding moved from M7 to **M0.8** (traceability tooling must predate the first annotated commit, not follow it); continuous fuzzing wired into CI from **M0.5** instead of a one-off M7 review; a mandatory recalibration checkpoint added at **M2.10** (first real effort data point); solo-founder critical-path risk (R-4) mitigated with an explicit bench-task list (§2); frozen-directory CI guard scheduled at **M0.5** with a re-check at **M5.7**; SITL validation coverage distributed across M2/M3/M4/M5 instead of backloaded to a single M7 push.
- §8 added: the repeatable per-milestone execution loop (DoR check → load WBS into TaskCreate → generate against docs → seven-lens review → update progress.md → DoD check).

**M0 remaining — now with task IDs (see IMPLEMENTATION_PLAN.md §5):**
- [ ] M0.1 CMakePresets.json
- [ ] M0.2 clang-format/clang-tidy/MISRA config
- [ ] M0.3 services/common + hello-service template
- [ ] M0.4 GCS panel-host framework + migrate prototype views
- [ ] M0.5 CI shell + frozen-dir guard + nightly fuzz stub + doc-link checkers
- [ ] M0.6 setup.sh v0
- [ ] M0.7 OQ-3 map spike
- [ ] M0.8 rtm-generator scaffold + @req lint

**Next session:** start M0.1–M0.3, per IMPLEMENTATION_PLAN.md §9.

---

## 2026-07-07 — FlightMD real-project inspection; assets imported; docs corrected (ADR-0018, Review R4)

**Done:**
- Inspected `C:\Users\akass\Downloads\flightmd` — FlightMD is not a plan, it's a real, live, MIT-licensed product (github.com/Praddyx15/FlightMD, deployed at flightmd.vercel.app/flightmd-api.onrender.com), materially ahead of what UAOP's docs assumed: schema v1.5 (not v1.0), 3 log formats incl. MAVLink `.tlog` (not 2), 8 analyzers incl. an unweighted ascent/recovery module, AI-optional with **Groq default** (not Claude-mandatory), validated against 50 real-world logs across 11 vehicle types.
- **Imported** (small, MIT, same author — no licensing question): `tests/data/sample_logs/` (6 clean/flawed fixture logs across `.ulg`/`.bin`/`.tlog` + generator script, ~1.5 MB) and `backend/services/parameter-engine/data/` (PX4 v1.13/v1.14 param defaults + safe-range JSON, ~7 KB). Both have provenance READMEs.
- **Deliberately not imported:** `flightmd_core` source/API/frontend (stays a pinned dependency per ADR-0001/REPOSITORY_STRUCTURE §2 rule 1, never vendored); the 8.5 GB real-world validation corpus (no Git LFS yet — recorded as a bench-task candidate, not a silent multi-GB commit).
- **ADR-0018** + **Review R4** (7 findings, F26–F32) in DECISIONS.md: AI-enhancement now structurally off-by-default in UAOP's integration (strengthens air-gap posture — this was a correction *toward* better security, not a new risk); `ASCENT_PROFILE` category filtered at the integration boundary; explicit boundary rule that UAOP never consumes FlightMD's own fleet-ops features; RISK_REGISTER R-6 narrowed to the UAOP-native LSTM tier only.
- Corrected: AI_ENGINE.md (v0.1.1), MASTER_CONTEXT.md §10, PRODUCT_REQUIREMENTS.md (UAOP-HLR-031 offline-by-default clause), RISK_REGISTER.md R-6, PROJECT_STRUCTURE.md (v0.1.2), TESTING.md (v0.1.1). IMPLEMENTATION_PLAN.md → v0.2.1: M2.3 gains tlog-replay fixtures, M4.4 marked de-risked, Phase 2/M9 forward-note added, bench-task list gains the LFS+corpus option.

**M0 status:** unchanged from 2026-07-06 — no M0 tasks executed this session, only the FlightMD inspection/import/doc-correction work above.

**Next session:** still start M0.1–M0.3, per IMPLEMENTATION_PLAN.md §9. When M9 (Phase 2 ai-engine) is eventually detailed, start from AI_ENGINE.md §2.1/§6 and ADR-0018 directly — the integration contract is now accurate, not aspirational.

---

## 2026-07-07 (later) — Upstream code-reuse policy decided (ADR-0019, Review R5)

**Trigger:** founder proposal to accelerate development by taking and refining code from ArduPilot, Mission Planner, QGroundControl, PX4-Autopilot, PX4-ECL.

**Decision (licenses verified against live repos):** three tiers. **PX4-Autopilot (BSD-3)** = port freely with provenance headers + THIRD_PARTY_NOTICES entries. **QGC (dual Apache-2.0/GPL-3.0)** = port under the Apache option with per-file verification — its MissionManager/ParameterManager state machines become the reference implementations for M4.2/M4.4 protocol code, ported into our Result<T,E> idiom. **ArduPilot + Mission Planner (GPL-3.0) = never in-tree in any form** — a single "refined" GPL function would relicense UAOP's distribution, deciding OQ-4 by accident and foreclosing Phase 4 closed components; they remain behavioral references (quirk tables, tuning benchmarks), study-only in external-resources/.

**Also caught:** PX4-ECL is archived (May 2024) — EKF2 lives in PX4-Autopilot mainline; source from there. New prompt-hygiene hard rule in CLAUDE.md/AGENTS.md: never paste GPL source into a code-generation prompt. M0.5 gains an in-PR license-allowlist gate + GPL-text denylist (release-time-only auditing would catch contamination months late).

**Files touched:** DECISIONS.md (v0.1.4 — ADR-0019 + Review R5, F33–F38), IMPLEMENTATION_PLAN.md (v0.2.2 — M0.5/M4.2/M4.4), MAVLINK_INTEGRATION.md (v0.1.1 — reference-implementation note), CLAUDE.md/AGENTS.md (hard rule), external-resources/README.md (rule 5).

**M0 status:** unchanged — policy work only. **Next session:** M0.1–M0.3.

---

## 2026-07-07 — DEVELOPMENT STARTED: M0 mostly complete in one session

**Toolchain discovered on this machine:** CMake 4.3.3, Qt 6.11.1 (mingw_64 kit), MinGW GCC 13.1 + Ninja (C:\Qt\Tools), Docker 29.3.1, Python 3.14 — full local build capability confirmed.

**Done (see IMPLEMENTATION_PLAN.md §M0 status block for detail):**
- [x] M0.1 CMakePresets.json — configure/build/test presets, sanitizer preset (Linux), gcs-debug preset
- [x] M0.2 .clang-format / .clang-tidy / compliance/tools/misra-config (with honest MISRA-checker-coverage note)
- [x] M0.3 `uaop::common` Result<T,E> + Error taxonomy (first `@req:` trace: UAOP-NFR-008) + `tools/service-template/` hexagonal example (link-monitor domain previewing HLR-004 semantics) — **built + 3/3 tests green locally, GCC 13, warnings-as-errors**
- [x] M0.5 CI: pr.yml (guards → linux x86_64 + arm64 build/test → Trivy), nightly.yml (asan + self-failing fuzz/SITL stubs), 4 guard scripts all green locally, THIRD_PARTY_NOTICES.md seeded
- [x] M0.8 rtm_gen.py — RTM.md generated (first real trace row), annotation lint enforces known-ID + template-quarantine rules
- [~] M0.6 compose.yaml (NATS JetStream/TimescaleDB/Redis/MinIO + sitl profile) + setup.sh — `docker compose config` VALID; live `up --wait` pending engine start
- [ ] M0.4 GCS panel-host framework ← **next session's main item**
- [ ] M0.7 map spike (OQ-3) — after M0.4

**Firsts:** first compiled UAOP code, first passing tests, first RTM with a real trace, first CI-enforceable guards.

**Calibration note for M2.10:** ~5.5 estimated focus-days of scaffolding landed in one session. AI-assisted authoring compresses file-authoring work heavily; integration/verification work (M1 onward) will not compress at the same ratio — recalibrate at M2.10 with both data points, not just this one.

**Next session:** M0.4 (panel host + migrate 4 prototype views), then M0.7 map spike; M0 DoD check; then M1.1 proto contracts.

**Post-push addendum (same day):**
- First cloud CI run failed on `aquasecurity/trivy-action@0.28.0` — their tags moved to a `v` prefix. Fixed by **SHA-pinning all actions** (`66ce154`), which CI_CD.md §6 had required all along; the original workflow violated our own supply-chain rule. Lesson logged: the rule existed, enforcement (a reviewer eye on `uses:` lines) did not — self-caught within one run.
- **Run 28813987888 fully green: guards ✅ · linux x86_64 build+test ✅ · linux ARM64 build+test ✅ · trivy ✅.** UAOP-NFR-007 (one pipeline, both architectures) is now continuously verified — the code that passed on Windows/MinGW passes on both Linux arches untouched.
- M0.6 live-up remains blocked locally: Docker Desktop engine did not come up after 7+ min of polling — likely waiting on a first-run GUI dialog (license/WSL2 prompt). **Founder action: open Docker Desktop once, accept any prompt**, then `bash tools/setup.sh` (Git Bash) or `docker compose -f infrastructure/docker/compose/compose.yaml up -d --wait` completes M0.6.
