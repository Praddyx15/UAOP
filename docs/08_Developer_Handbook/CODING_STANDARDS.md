# CODING_STANDARDS

**Version 0.1.1 · 2026-07-07 · Enforced by CI (CI_CD.md), not by exhortation. Applies to all code identically, whatever tool or method drafted it — no exemptions either direction.**

## 1. C++ (core services, bridges, GCS backend)

**Language & build (ADR-0016):** services and bridges **C++17**; GCS frontend **C++20 on Qt 6.8 LTS** (DAL D-equivalent, not MISRA-bound — ranges, designated initializers, and coroutines are welcome there). One toolchain: GCC 12+/Clang 16+. Warnings-as-errors (`-Wall -Wextra -Wconversion`), clang-format (repo `.clang-format`, LLVM-derived, 100 col) — formatting is machine-owned, never discussed in review.

**Safety tiers (COMPLIANCE.md §A.2):**

| Rule | DAL C-equiv paths | Elsewhere |
|---|---|---|
| **MISRA C++:2023** ruleset (Cppcheck config; edition per ADR-0016) | Zero critical, gate-blocking | Advisory report |
| Exceptions | **Forbidden** — `Result<T,E>` (`services/common/`), `[[nodiscard]]` enforced | Allowed internally; converted to Result/status at service boundaries |
| Dynamic allocation | **Forbidden on RT paths** — pre-allocated pools, fixed-size rings (ADR-0013) | Normal RAII |
| Traceability | `@req:`/`@test:` per function, lint-gated (UAOP-NFR-009) | Encouraged |

**Idiom canon:** RAII everywhere; ownership explicit (`unique_ptr` default, raw pointers = non-owning observers only, documented); no naked `new/delete`; `std::variant`/`optional` over sentinel values (proto3 absence discipline starts in code); all public service interfaces defined by proto contracts — no ad-hoc structs across boundaries; concurrency via the patterns in SOFTWARE_ARCHITECTURE.md §4 only (introducing a new threading model requires an ADR); includes IWYU-clean; namespaces `uaop::<domain>`.

## 2. Python (ai-engine, simulation, compliance tooling)

3.11+; ruff (lint+format) + mypy `--strict` on service code; Pydantic v2 models at all I/O boundaries; async-first in services with CPU work in process pools (AI_ENGINE.md §3); dependencies pinned via lockfile, reviewed like code (SECURITY.md §7); `flightmd_core` import discipline — pinned version, never vendored, never forked (ADR-0014); no `# type: ignore` without an issue reference.

## 3. QML / GCS

Views bind to C++ models only; zero business logic in QML (UI_GUIDELINES.md §6); qmllint + qmlformat gated; token references only for colors/fonts/spacing (STYLE_GUIDE.md §9 — literal hex in QML fails lint); every user-visible string wrapped in `qsTr()`; panel modules follow the manifest shape (UI_GUIDELINES.md §6).

## 4. Comments & naming (constitutional style, Master Context UI direction)

- **Concise identifiers, modular structure, minimal comments:** code explains itself by naming and shape; comments carry only what code cannot — invariants, units, safety constraints, protocol quirk citations ("ArduPilot 4.5 ACKs with…" + issue link), and `@req:` traceability.
- Forbidden comment classes: narration ("increment counter"), changelog archaeology (git owns history), commented-out code (delete it), TODO without an issue ID.
- Units in names or types, always: `timeout_ms`, `alt_agl_m`, `Radians` wrapper types on trig boundaries — a unit bug in this domain bends airframes.
- Avoid repetitive AI-style boilerplate (constitution): if three services grow the same 40 lines, that's a `services/common/` candidate, not a copy-paste pattern.

## 5. Error handling & logging discipline

Every error mapped to the TRANSIENT/INVALID/FATAL taxonomy at the boundary where it's decided (SOFTWARE_ARCHITECTURE.md §5); no swallowed errors — minimum is a counter + structured log; no `catch(...)`/`except Exception` without rethrow-or-taxonomy; log calls follow LOGGING.md (structured fields, stable `event` ids, no hot-path allocation); user-facing error text follows UX_GUIDELINES.md §6.

## 6. Review standards

A PR is reviewable when: single concern, green pipeline, description states *what changed and why* referencing requirement/issue ids, and contract changes flagged loudly. Reviewers hold the line on: architecture drift (undeclared dependencies — the CI graph check backs this, CI_CD.md §5), idempotency of new consumers, absence/staleness handling on any new UI-bound data, failure-mode handling matching the relevant document's table, and test presence per TESTING.md §2.1. "Works" is not a review standard; "matches the documented design or updates the document" is.

## 7. Git hygiene

Conventional commit prefixes (`feat/fix/docs/refactor/test/build` + service scope: `feat(mission-engine): …`); trunk-based, short-lived branches (CI_CD.md §3); no force-push to shared branches; generated artifacts never committed (proto codegen, build output); binary test data via LFS in `tests/data/` with provenance notes.
