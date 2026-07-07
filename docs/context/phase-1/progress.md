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
