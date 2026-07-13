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

**Session 2 (2026-07-07) — M0.4 complete:**
- [x] M0.4 panel-host framework: PanelRegistry/WorkspaceManager (src/core), STYLE_GUIDE Theme tokens + legacy aliases, Shell.qml (workspace switcher + status chips), PanelHost (SplitView columns, tabbed stacks), floating-copy tear-off v0, `--selfcheck` smoke test in ctest. 4/4 tests green locally. Debt recorded: views still use legacy Theme aliases (re-theme M2/M6); tear-off is copy-based until M6's move-to-float docking; fonts are system fallbacks until Inter/JetBrains Mono are bundled.
- Attribution policy enforced this session per the standing instruction: history rewritten (sole-author commits), docs reframed to AI-assisted/tooling-neutral wording, `.claude/settings.local.json` disables co-author trailers.
- M0 DoD nearly met: remaining = M0.7 map spike + M0.6 live infra-up (Docker Desktop first-run click).

**Post-push addendum (same day):**
- First cloud CI run failed on `aquasecurity/trivy-action@0.28.0` — their tags moved to a `v` prefix. Fixed by **SHA-pinning all actions** (`66ce154`), which CI_CD.md §6 had required all along; the original workflow violated our own supply-chain rule. Lesson logged: the rule existed, enforcement (a reviewer eye on `uses:` lines) did not — self-caught within one run.
- **Run 28813987888 fully green: guards ✅ · linux x86_64 build+test ✅ · linux ARM64 build+test ✅ · trivy ✅.** UAOP-NFR-007 (one pipeline, both architectures) is now continuously verified — the code that passed on Windows/MinGW passes on both Linux arches untouched.
- M0.6 live-up remains blocked locally: Docker Desktop engine did not come up after 7+ min of polling — likely waiting on a first-run GUI dialog (license/WSL2 prompt). **Founder action: open Docker Desktop once, accept any prompt**, then `bash tools/setup.sh` (Git Bash) or `docker compose -f infrastructure/docker/compose/compose.yaml up -d --wait` completes M0.6.

---

## 2026-07-07 (session 3) — M0.7 map spike resolved: ADR-0020, Review R6

**Investigated first, built second** (per the spike's actual purpose): verified MapLibre Native Qt's real compatibility against the local toolchain rather than assuming it. Finding: prebuilt binaries top out at **Qt 6.5.3/6.6.3/6.7**; the local dev kit is **Qt 6.11.1** — no ABI-safe prebuilt, and a from-source build (submodules, multi-stage CMake, historically Linux/macOS/mobile-first) would have burned the whole timebox on a dependency that gates nothing else in Phase 1.

**Decision (ADR-0020):** ship an interim raster renderer now, defer full MapLibre integration to its own future spike (tracked as a bench-task candidate, same status as the FlightMD real-log-corpus/LFS item).

**Built:**
- `SlippyMath.h` — pure lon/lat↔tile-fraction Web Mercator math, zero Qt dependency, 5 unit tests (`gcs.slippy_math`).
- `MbtilesProvider` — `QQuickImageProvider` backed by `QSqlDatabase("QSQLITE")` (bundled with Qt, no new dependency); parameterized queries (verified no SQL-injection surface despite QML-originated input — R6/F42); malformed/missing tiles degrade to a labeled placeholder, never null/crash. 4 assertions (`gcs.mbtiles_provider`).
- `MapView.qml` — minimal pan/zoom tile-grid view, registered as the ordinary `"map"` panel through the existing `PanelRegistry` — **the M0.4 panel-host framework needed zero changes**, which is the real validation of that framework's design.
- `tests/data/map/world.mbtiles` — synthetic 85-tile test set (zoom 0–3, ~104 KB) + `make_test_mbtiles.py` generator; committed directly, no LFS (same size-based reasoning as the FlightMD fixtures).

**Bug caught and fixed during this work (R6/F39):** the first test run crashed with an opaque OS exit code (0xC0000602) and zero diagnostic output. Root cause: `MbtilesProvider`'s placeholder-tile path calls `QPainter::drawText()`, which needs a font/platform backend — the test harness used a bare `QCoreApplication` instead of `QGuiApplication`. Fixed by matching the `gcs.selfcheck` pattern (`QGuiApplication` + `QT_QPA_PLATFORM=offscreen`). Logged as a general lesson: GUI-touching unit tests need a real `QGuiApplication`, not just `QCoreApplication` — a silent, opaque crash is worse than a clean assertion failure.

**Result: 6/6 GCS tests green** (common.result, template.link_monitor, template.runs, gcs.selfcheck, gcs.slippy_math, gcs.mbtiles_provider). GCS launched and visually confirmed running with the new map panel in the FLIGHT OPS workspace.

**Docs updated:** ADR-0020 + Review R6 (5 findings, F39–F43) in DECISIONS.md (v0.1.5); OQ-3 marked resolved there and in MASTER_CONTEXT.md §12; RISK_REGISTER R-11 updated (materialized-as-expected, residual risk narrowed, score 6→4); IMPLEMENTATION_PLAN.md v0.2.3 (M0.7 row + M0 status block).

**M0 status: Definition of Done met except the M0.6 live-infra check**, which only needs the founder's one-time Docker Desktop click. Every engineering item in M0 is otherwise complete.

---

## 2026-07-07 (session 3, cont'd) — build-gcs-linux CI job: two fix-forward iterations to green

R6/F44's new CI job (`build-gcs-linux`, provisions Qt via aqtinstall) failed twice before going green — both genuine findings, not process failures, resolved in the same session per the established fix-forward discipline:

1. **Attempt 1 failed:** `cmake --preset gcs-debug` configure step failed immediately. Root cause: `qtshadertools` is a separate addon module — confirmed via `aqt list-qt --modules` rather than guessed — that Qt6's `Quick` CMake package config depends on even for plain QML apps doing no custom shader work. Added `-m qtshadertools` to the aqt install command.
2. **Attempt 2 failed at the identical step**, meaning the first fix was necessary but not sufficient. Read aqt's own source (`QtRepoProperty.get_arch_dir_name` in `metadata.py`) rather than guessing again: the `linux_gcc_64` arch argument passed to `aqt install-qt` is an *install-request* identifier, not the on-disk directory name — aqt writes the kit to a directory named **`gcc_64`** (prefix stripped), exactly analogous to our local Windows kit living at `mingw_64` with no OS prefix. `CMAKE_PREFIX_PATH` had been pointing at a directory that never existed. Fixed by **detecting** the installed kit directory at runtime (with a `lib/cmake/Qt6` sanity check that fails loudly) instead of hardcoding aqt's internal naming — the same "verify against reality, don't assume compatibility" discipline as ADR-0020/F40, applied here to CI plumbing instead of a third-party library.
3. **Attempt 3: all 5 jobs green** (guards, x86_64, ARM64, trivy, **GCS**). Run `28860006158`.

**Lesson for future CI additions:** when wiring a new toolchain into CI, verify its actual installed layout/dependencies against source or direct experimentation rather than assuming documentation or naming conventions hold — this is now the second time in one session (MapLibre Qt-version check, aqt directory naming) that "verify, don't assume" caught a real gap before it became a stale, permanently-broken CI job.

**M0 status unchanged from above: DoD met except the M0.6 Docker click.**

---

## 2026-07-07 (session 3, cont'd) — M0.6 closed: Docker Desktop environment issue resolved; M0 fully complete

The "one Docker click" turned out to be a genuine local-environment bug hunt, not a first-run consent dialog. Recorded in full since it cost real time and the fix is non-obvious if it recurs on another machine:

**Symptom:** Docker Desktop crashed on every launch attempt with variations of `initializing <component>: listening on unix://...: remove ...: The file cannot be accessed by the system.` — first on `Inference Manager` (`dockerInference`), then on `Secrets Engine` (`engine.sock`, a stale file dated **2026-05-25**, six weeks old).

**What did NOT fix it** (all tried, all failed, each a real data point):
1. Deleting the stale socket file as Administrator — worked once, but Docker recreated an identical broken file on the very next launch.
2. Docker Desktop's own "Reset to factory defaults" — the stale files lived outside Docker's own data tree (`%LOCALAPPDATA%\docker-secrets-engine\`, not `%LOCALAPPDATA%\Docker\`), so the reset didn't touch them.
3. Setting `EnableDockerAI: false` in `settings-store.json` — the Inference Manager subsystem starts regardless of that flag; it controls a different feature (likely the "Ask Gordon" chat UI).

**Root cause:** Windows Defender real-time protection was intercepting reparse-point (Unix-socket-emulation) operations under `AppData\Local\Docker\` and `AppData\Local\docker-secrets-engine\` — Docker could create these special socket files but not delete/replace them on the next startup, and neither could an elevated `Remove-Item` or `fsutil reparsepoint delete` (both failed with the identical OS-level error even as Administrator, which was the tell that this wasn't an ordinary permissions problem).

**Fix:** `Add-MpPreference -ExclusionPath 'C:\Users\akass\AppData\Local\Docker','C:\Users\akass\AppData\Local\docker-secrets-engine'` (elevated), then one final elevated deletion of both stale files, then relaunch. **Engine came up clean and stayed stable.**

**M0.6 completed:** `bash tools/setup.sh up` — NATS, TimescaleDB, Redis, MinIO all `Up ... (healthy)`, confirmed stable 3+ minutes later via `docker compose ps`. Build re-verified (3/3 tests), all 4 CI guards re-verified green.

**M0 — Definition of Done: fully met. All eight tasks (M0.1–M0.8) complete.** IMPLEMENTATION_PLAN.md → v0.2.5.

**Next session:** begin **M1 — Contracts and platform plumbing** (proto schemas, `services/common` runtime library additions, NATS stream provisioning, api-gateway v0, DB migration harness) — see IMPLEMENTATION_PLAN.md §"M1".

---

## 2026-07-07 (session 4) — Pre-M1 bench task: Poppins bundled as the platform UI font

Founder decision (2026-07-07): Poppins (Light/Regular/Medium/Bold, each with italic) is the GCS brand font, replacing the earlier Inter/system-fallback placeholder in STYLE_GUIDE.md §3. Not on the M1 WBS — handled as a contained bench task ahead of it, same pattern as prior quick founder-driven UI decisions.

**Done:**
- All 8 static Poppins TTF faces + `OFL.txt` (SIL Open Font License 1.1) fetched from `google/fonts` and placed under `frontend/qt-desktop-gcs/resources/fonts/Poppins/`, with a provenance `README.md`.
- Embedded as Qt resources (`qt_add_resources` in CMakeLists.txt) and registered via `QFontDatabase::addApplicationFont` in `main.cpp` — never a loose-file or system-font dependency, consistent with the air-gap posture (UAOP-NFR-001).
- `Theme.qml` updated: `uiFontFamily` → `"Poppins"`, new weight tokens (`weightLight`/`weightRegular`/`weightMedium`/`weightBold`) and two new font tokens (`bodyItalicFont`, `emphasisFont`). Numeric telemetry stays on the existing mono fallback (`Consolas`) — unrelated to this pass, tracked separately (bundling JetBrains Mono the same way).
- STYLE_GUIDE.md (v0.1.1), PROJECT_STRUCTURE.md, THIRD_PARTY_NOTICES.md updated with the font entry and rationale.
- Build verified clean after fixing a linker `Permission denied` (a previously-launched `uaop_gcs.exe` still held the file locked — killed the process, rebuild succeeded). **6/6 tests green.**
- Font-load verified by running the `--selfcheck` binary directly (bypassing ctest's stdout suppression on pass): exit 0, **zero `qWarning` output** — `loadBundledFonts()` prints a named warning per face on failure, so silence across all 8 `addApplicationFont` calls confirms every face registered.
- **Bug caught by the doc-link CI guard**: the new Poppins `README.md` had an off-by-one relative path to `STYLE_GUIDE.md` (`../../../../docs/...`, needed 5 `../` not 4 from that directory depth). Caught by `check_doc_links.py`, fixed, all 4 CI guards re-verified green (doc-links, licenses, frozen-dirs, RTM annotation lint).

**Also received:** a reference screenshot (drone motor/prop-calculator gauge UI) flagged by the founder as potentially useful — filed for future reference against UI_GUIDELINES.md §3 (Motor Test / Vehicle Config panel work), not actioned yet since no such panel exists before M1's contracts land.

**Next:** M1 — Contracts and platform plumbing, starting with M1.1 (telemetry v1 proto).

---

## 2026-07-07 (session 4, cont'd) — M1.1: telemetry v1 proto contract frozen

`api/proto/uaop/telemetry/v1/` written by hand against TELEMETRY_ENGINE.md §2's full field census and ADR-0009's freeze decision — the foundational contract everything else in M1 depends on.

**Built:**
- `vehicle_telemetry.proto`: `TelemetryEnvelope` (vehicle_id, per-vehicle monotonic sequence, vehicle/bridge timestamps, sim flag) + all 13 categories (position, attitude, velocity, imu, ekf, battery, esc, rc, link, rf, health, state, remoteid) as nested message types, `VehicleTelemetry` wrapping them in a `oneof category` — one populated branch per NATS subject publish, sharing the envelope for uniform gap accounting. proto3 `optional` on every field where absence is meaningful (no fix, no RTK block, sensor not installed), per ADR-0009's "absent ≠ zero" rule. Units suffixed into field names throughout (`_deg`, `_mps`, `_mps2`, `_radps`, `_gauss`, `_v`, `_a`, `_c`) per CODING_STANDARDS §4.
- `snapshot.proto` (`TelemetrySnapshot`, flattened fleet-card summary) and `delta.proto` (`TelemetryDelta`, same category oneof + `google.protobuf.FieldMask` for changed-fields-only transmission on constrained links) — both same v1 schema version, additive-only.
- `buf.yaml` (v2 module, `STANDARD` lint, `FILE`-level breaking rules) and `buf.gen.yaml` (`protoc_builtin` C++/Python message codegen — gRPC plugins join once `gateway/v1` services land at M1.3). Installed `buf` 1.71.0 locally via `winget` to verify rather than assume.
- **`buf lint`: clean.** **`buf breaking` verified against reality**: snapshotted the proto tree, deliberately renumbered `Attitude.roll_deg` from 1→99, confirmed `buf breaking` failed loudly (exit 100, correct diagnostic), reverted — the tool actually catches what ADR-0009 requires it to catch, not just configured and assumed.
- New `proto-contract` CI job in `pr.yml`: `bufbuild/buf-setup-action` (SHA-pinned, v1.50.0) → `buf lint` → `buf breaking --against main`, with a one-time bootstrap guard (`git cat-file -e main:api/proto/buf.yaml`) so the PR that first introduces the module doesn't fail against a baseline that doesn't exist yet — every subsequent PR gets a real diff.

**Docs updated:** IMPLEMENTATION_PLAN.md v0.2.7 (M1 status block), PROJECT_STRUCTURE.md v0.1.3 (`api/proto/` entries no longer bare `⊕`). All 4 CI guards re-verified green after the change.

**Next:** M1.2 — `EventEnvelope` + platform error-code registry proto.

---

## 2026-07-07 (session 4, cont'd) — CI verified green, M1.2 done: EventEnvelope + error registry

M1.1's push was verified against real CI, not assumed: run [28877934227](https://github.com/Praddyx15/UAOP/actions/runs/28877934227) — all 6 jobs (guards, x86_64, ARM64, GCS, trivy, and the new **proto-contract** job) green.

**M1.2 built:**
- `api/proto/uaop/events/v1/envelope.proto` — `EventEnvelope`, copied verbatim from EVENT_FLOW.md §4's already-frozen shape (event_id, schema, sequence, occurred_at, source_service, vehicle_id, correlation_id, actor, sim, payload).
- `api/proto/uaop/errors/v1/errors.proto` — `ErrorCode` enum registry (API_SPECIFICATION.md §1's single error envelope) + `Error` message. **Placement deviation, documented:** API_SPECIFICATION.md originally sketched an unversioned `api/proto/errors.proto`; PROJECT_STRUCTURE.md's tree also showed it unversioned directly under `uaop/`. Put it at `uaop/errors/v1/` instead (package `uaop.errors.v1`) because an unversioned single-segment package fails buf's `PACKAGE_DIRECTORY_MATCH` lint rule and breaks the versioning convention every other package in the tree follows — a real constraint discovered by running the linter, not a stylistic preference. API_SPECIFICATION.md §1 and PROJECT_STRUCTURE.md updated to the actual path (doc/code disagreement is a P1 defect per CLAUDE.md).
- Enum values are prefixed `ERROR_CODE_*` (buf's `ENUM_VALUE_PREFIX` rule) — e.g. `ERROR_CODE_VEHICLE_NOT_READY`, not API_SPECIFICATION.md's shorthand `VEHICLE_NOT_READY` example. proto3 JSON mapping renders the full enum name, so that's the exact wire string clients receive; noted as a deliberate, lint-driven deviation from the doc's illustrative shorthand.
- `buf lint`: clean. `buf breaking --against main`: clean (pure additions, exit 0).

**Docs updated:** IMPLEMENTATION_PLAN.md v0.2.8, PROJECT_STRUCTURE.md v0.1.4, API_SPECIFICATION.md §1 path fix. All 4 CI guards + buf lint re-verified green.

**Next:** M1.3 — `api/proto/uaop/gateway/v1/` (command/mission/parameter/telemetry_query/log/compliance services).

---

## 2026-07-07 (session 5) — DO-178C folder scaffold, M1.3, M1.4: services/common runtime

Founder request: proceed through M1.3–M1.8, and set up a folder for HLR/LLR/DO-178C artifacts per a reference DO-178C Annex A table infographic. Handled the folder request and M1.3/M1.4 this session; M1.5–M1.8 continue next.

**DO-178C folder:** `docs/compliance/DO-178C/A-2_Planning/` through `A-7_Verification_of_Outputs/`, each README-indexed against the real DO-178C Annex A tables, cross-referencing where the substance already lives (PRODUCT_REQUIREMENTS.md *is* the HLR set; RTM.md is the traceability matrix) and honestly marking what's genuinely not authored yet (PSAC, SDP, SQAP, LLRs — none exist before the first DAL C-equivalent module does). This operationalizes COMPLIANCE.md §A.3's already-stated plan rather than inventing new policy.

**M1.3 — gateway/v1 gRPC services:** `command.proto`, `mission.proto`, `parameter.proto`, `telemetry_query.proto`, `log.proto`, `compliance.proto` — six services matching API_SPECIFICATION.md's REST resource map and async-command semantics. First `buf lint` run caught 15 `RPC_RESPONSE_STANDARD_NAME` violations (reusing a resource message directly as an RPC response, or reusing one response type across two RPCs) — fixed by wrapping every RPC response in its own message, the pattern buf's STANDARD ruleset expects. `buf lint`/`buf breaking` clean after the fix.

**M1.4 — services/common runtime additions**, all real and unit-tested rather than stubbed: UUIDv7 (`uuid.h`), monotonic per-key sequencing (`sequence.h`), a structured JSON logger matching LOGGING.md §2's schema exactly (`logger.h/.cpp`), the layered config loader (`config.h/.cpp` — compiled defaults → file → env, a deliberately-restricted YAML-compatible subset rather than a new yaml-cpp dependency), Prometheus-format metrics (`metrics.h/.cpp`), a health-check registry (`health.h/.cpp`), and a dependency-free `GET /healthz` + `GET /metrics` HTTP listener over raw sockets (`health_metrics_server.h/.cpp`, Windows/POSIX both — chose not to pull in Drogon for two GET routes when no service exists yet to justify the framework).

**The NATS JetStream client — honest gap, not a silent skip:** `jetstream.h` defines the `JetStreamPublisher`/`JetStreamConsumer` port (hexagonal pattern, same shape as the M0.3 service-template's `EventPublisher`); `in_memory_jetstream.h/.cpp` is a real, tested single-process adapter with redelivery simulation; `dedup.h` is a TTL dedup set matching DATABASE.md §6's Redis pattern. No adapter here talks to a real NATS server — vendoring `nats.c` (OpenSSL linkage, MinGW cross-platform build) is genuine unverified work that deserves its own timeboxed spike, the same judgment call ADR-0020 made for MapLibre Native Qt rather than attempting it blind and burning the session on a build-toolchain rabbit hole. M1.5/M1.6/M1.8 are scoped to talk to the real NATS server directly (CLI / Node.js / Python) specifically so M1 isn't blocked on that spike landing first.

**Build notes:** `services/common` converted from a header-only `INTERFACE` library to a compiled `STATIC` one (six new `.cpp` files). Two real compile bugs caught and fixed by actually building rather than assuming: a missing `<cstdint>` include, and an ambiguous single-pair `ConfigStore({{...}})` constructor call in tests (GCC's brace-init overload resolution — worked around by explicitly typing the `unordered_map` at the call site) — plus one real test bug, not an implementation bug: a UUIDv7 ordering assertion that assumed strict ordering across calls with no time gap, which UUIDv7 doesn't promise (only ordering across distinct milliseconds is guaranteed). **9/9 tests green.**

**Next:** M1.5 — JetStream stream provisioning against the real compose NATS instance.

---

## 2026-07-12 (session 6) — Docker Desktop crash loop (new root cause), M1.5, M1.6

**Docker Desktop crashed again**, same symptom family as the M0.6 saga (stale `unix://` socket files: `dockerInference`, then `docker-secrets-engine/engine.sock`) but this time with a genuinely new root cause underneath. Docker Desktop's own log (`AppData/Local/Docker/log/host/com.docker.backend.exe.log`) showed the real mechanism: on startup it runs a "lingering processes" self-check, tries to kill leftover `Docker Desktop.exe` processes from the previous launch, and **crashes itself if that kill doesn't fully succeed** — leaving its own new processes as the *next* launch's lingering processes. Each partial cleanup (killing some but not all stale processes) fed the next crash; the count climbed 6 → 4 → 2 across several rounds of "quit, run the fix, relaunch" before we got it. Fix: kill every `Docker Desktop.exe` process until the count is verified to be exactly zero, *then* launch once — a genuine zero, not "close enough," breaks the loop. Also found: the specific `dockerInference` file recurs because it belongs to Docker's AI/Inference Manager feature, unrelated to anything this project needs — recommended the founder disable it via Settings to stop the recurrence rather than keep firefighting it.

**M1.5 — JetStream stream provisioning:** `tools/provision-streams.sh`, idempotent, run for real against the live NATS server once Docker recovered. Verified the nats CLI's actual `stream add` flags via `--help` rather than assuming (found `--persist-mode` only accepts `default`/`async` — no true always-fsync option, so ADR-0015's AUDIT durability guarantee needs a server-level JetStream config change this script can't provide; documented, not silently dropped). All 5 streams (TELEMETRY/EVENTS/COMMANDS/AI/AUDIT) created and confirmed via `nats stream info` with the exact retention/limits EVENT_FLOW.md §2 specifies.

**M1.6 — api-gateway v0:** `backend/api-gateway/` (Node 22, ESM). JWT auth (six-role RBAC, ≤60min tokens) + Redis-backed denylist; the WebSocket subscription protocol exactly per API_SPECIFICATION.md §5 (subscribe op, `stream`/`seq`/`ts`/`payload` frames, explicit `gap` frame under `rate_hz` shaping, 5s heartbeat); gRPC forwarding to `VehicleCommandService` via `@grpc/proto-loader` dynamic loading (no C++-style codegen step needed — loads the real `.proto` files at runtime). **11/11 tests green, all against real infrastructure**: real Redis for the denylist, real NATS for the WebSocket relay (publishing raw messages on the just-provisioned TELEMETRY subject and asserting correct relay + rate-shaped gap accounting), and a real gRPC server built from the actual `command.proto` (standing in for vehicle-manager, which doesn't exist until M3+) for the forwarding tests. Two real bugs caught by actually running the suite: Node 22's `--test` flag doesn't take a directory positional the way `--test tests/` implies (fixed to bare `node --test`, which auto-discovers `*.test.js`), and a copy-paste path-depth bug in the test file's own proto-root calculation (`tests/` is one directory shallower than `src/grpc/`, so needed 3 `../` not 4).

**Docs updated:** IMPLEMENTATION_PLAN.md v0.2.10, PROJECT_STRUCTURE.md v0.1.6.

**Next:** M1.7 — Postgres/Timescale schema baseline + migration harness.

---

## 2026-07-13 (session 6, cont'd) — M1.7, M1.8: M1 complete

**M1.7 — Postgres/Timescale schema baseline:** `tools/db-migrate.sh` (idempotent runner, tracked in `public.schema_migrations`) + 10 numbered files under `infrastructure/postgres/migrations/`, covering all 9 DATABASE.md §2 schemas: `mission`/`param`/`compliance`/`flightlog`/`vehicle` with per-service DB roles (schema-scoped grants only, per DATA_FLOW.md §5's ownership matrix — a role never touches a schema it doesn't own); `audit.chain` as a partitioned, append-only table with a BEFORE INSERT trigger enforcing hash-chain continuity in the database itself, not just trusted to the application; `auth`/`node` schemas created but deliberately left ungranted (no owning service exists in PROJECT_STRUCTURE.md's list yet — an honest gap, not a guessed role name); and all 13 telemetry hypertables, column-for-column matching the M1.1 proto census, with 1h chunking, 24h compression policy, and 4 representative continuous aggregates (position/attitude/battery/health).

Run for real against the live compose Postgres/TimescaleDB, not just written and assumed correct. One genuine bug: `CURRENT_DATE + INTERVAL '1 month'` in Postgres returns a `timestamp`, not a `DATE` — broke a partition-creation function expecting `DATE`, fixed with an explicit `::DATE` cast. Everything else (10 migrations, ~30 tables, the compression DO-loop, the continuous aggregates) applied clean on the first real run after that one fix.

**Verified the audit chain trigger actually works**, not just that it compiles: inserted a valid first entry (`prev_hash` NULL), then a deliberately broken second entry (wrong `prev_hash`) — rejected with the expected integrity-violation error — then a correctly-chained third entry, which succeeded. Exactly the behavior ADR-0015/COMPLIANCE.md §B.3 require, proven against a live database rather than asserted from the DDL alone.

**M1.8 — idempotency test harness:** `tests/integration/idempotency/` (Python, `nats-py`, pytest-asyncio) against the real M1.5-provisioned streams — duplicate-delivery (JetStream's own `Nats-Msg-Id` dedup window, asserted via `PubAck.duplicate`), gap-detection (publish app-level sequence 1,2,3,5, assert a consumer locates the missing 4), and out-of-order (publish 3,1,2, assert delivery arrives in *publish* order rather than *sorted* order — proving why EVENT_FLOW.md §3's "order by the envelope's sequence" rule exists at all).

**A real, non-obvious bug**, found by writing a standalone diagnostic script rather than guessing at the failure from pytest's traceback alone: all four `pull_subscribe`-based tests came back with zero messages, no exception, no timeout — `fetch()` just returned `[]`. The diagnostic (`js.stream_info` right after publishing) showed `StreamState(messages=0, ...)`: the messages had already been discarded. Cause: TELEMETRY is provisioned with **interest-based retention** (EVENT_FLOW.md §2, `--retention=interest` in `provision-streams.sh`) — a message with no registered consumer interest at publish time is eligible for immediate removal. The tests published first and created the durable consumer after, which is exactly backwards for that retention policy; real services never hit this because telemetry-engine's durable consumer is already running before any vehicle ever publishes. Fixed by subscribing before publishing in both affected test files — runtime dropped from 40s (three tests each waiting out a full 10s fetch timeout against an empty stream) to 0.14s once messages were actually being delivered. **6/6 tests green** after the fix.

**M1 — Definition of Done: fully met.** All eight tasks (M1.1–M1.8) complete, every one verified against real infrastructure (NATS, Redis, Postgres/TimescaleDB, and a real gRPC server standing in for vehicle-manager) rather than mocks — this session alone surfaced and fixed five genuine bugs (a Postgres type-coercion gotcha, an interest-retention test-ordering bug, a Node test-runner flag misunderstanding, a copy-paste path-depth bug, and the earlier buf lint RPC-naming violations) purely by actually running things instead of trusting the code on inspection. Two documented, deliberate gaps carry forward: the real NATS C++ client (M1.4) and AUDIT's true always-fsync guarantee (M1.5) — both real follow-up work, not silently dropped.

**Docs updated:** IMPLEMENTATION_PLAN.md v0.2.11 (M1 DoD marked met), PROJECT_STRUCTURE.md v0.1.7.

**Next:** M2 — First telemetry vertical slice (the credibility milestone).
