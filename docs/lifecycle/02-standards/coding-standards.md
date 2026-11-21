# Coding standards (pointer)

Issue 1, 2026-11-21. The coding rules already exist in the repository and
are not repeated here.

| Language | Rules | Enforced by |
|---|---|---|
| C++ (services, common library, desktop station) | `docs/08_Developer_Handbook/CODING_STANDARDS.md`; `CONTRIBUTING.md` (MISRA C++:2023 on flight-influencing code, no exceptions or dynamic allocation on real-time paths, units in names) | `.clang-format`, `.clang-tidy`; `compliance/tools/misra-config/` (Cppcheck configuration) |
| Protobuf | `CONTRIBUTING.md` (fields are only ever added) | `buf lint` and `buf breaking` in `.github/workflows/pr.yml` |
| Python | `CONTRIBUTING.md` (3.11+, type hints) | Tests only; no linter in CI was found |
| JavaScript (API gateway) | None written | Tests only; no linter in CI was found |
| QML | `docs/04_UI_Architecture/STYLE_GUIDE.md`, `UI_GUIDELINES.md` | `tools/gen_theme_tokens.py --check` for design tokens |

Gaps, for the later phase: a short written standard for JavaScript and
Python, linters for both in CI, and a check that the MISRA configuration is
actually run on every pull request (it was not seen in the workflow steps
read; to confirm). These are listed in
`docs/lifecycle/00-project-record/gap-analysis.md` and are not done here.
