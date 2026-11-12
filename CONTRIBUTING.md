# Contributing

Thanks for helping. UAOP runs next to real aircraft, so changes follow a few rules that keep it
traceable, offline-capable and safe to reason about. Most of them are enforced in CI.

## The rules

- **Read the design first.** Start with
  [MASTER_CONTEXT.md](docs/00_Project_Foundation/MASTER_CONTEXT.md), then
  [DECISIONS.md](docs/00_Project_Foundation/DECISIONS.md) and the numbered area in `docs/` for
  your change. The current milestone is in
  [IMPLEMENTATION_PLAN.md](docs/07_Implementation_Guides/IMPLEMENTATION_PLAN.md) §0.
- **The stack is fixed** by the ADRs in DECISIONS.md (ADR-0002, ADR-0005, ADR-0016, ADR-0022). A
  new technology needs an ADR first.
- **`api/proto/` is the contract.** Protobuf changes only ever add fields; `buf lint` and
  `buf breaking` run on every pull request.
- **Contract before code.** A service gets its entry in
  [MICROSERVICES.md](docs/03_Software_Architecture/MICROSERVICES.md) and its `.proto` before its
  first line of code. Start new services from `tools/service-template/`.
- **Both autopilots** (ADR-0023). Anything on the flight-evidence path works for PX4 and ArduPilot
  in the same change, or it isn't done.
- **Offline, always.** No feature may depend on the internet (UAOP-NFR-001). Optional network
  features are off by default.
- **AI advises, people decide** (ADR-0012). Model output carries a confidence score, is logged, and
  never sends a command.
- **Flight-influencing C++** follows MISRA C++:2023, uses no exceptions or dynamic allocation on
  real-time paths (ADR-0013), and carries `@req:` annotations so the traceability matrix stays
  complete. Every command is persisted as an audit event before it is acted on (ADR-0015).
- **Licences are tiered** (ADR-0019). PX4 (BSD) and QGroundControl (Apache option, checked per
  file) code may be ported with a provenance header and an entry in
  [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). **ArduPilot and Mission Planner are GPL-3.0:
  their code never enters this repository in any form**, and their source is never pasted into a
  code-generation tool. Use them as behavioural references only.
- **Frozen folders:** `backend/services/dji-service/` and `backend/services/fpv-service/` stay
  empty until OQ-1 is decided. CI fails on anything else in them.
- **Stay in the milestone.** Work belongs to the current milestone in
  [IMPLEMENTATION_PLAN.md](docs/07_Implementation_Guides/IMPLEMENTATION_PLAN.md); anything else
  waits or gets its own ADR.

## Setting up

```bash
git clone https://github.com/Praddyx15/UAOP.git && cd UAOP
./tools/setup.sh up            # infrastructure in Docker, then build, test and run the guards
cmake --preset gcs-debug -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64   # desktop GCS
```

## Before a pull request

```bash
python tools/ci/check_frozen_dirs.py
python tools/ci/check_doc_links.py
python tools/ci/check_licenses.py
python compliance/tools/rtm-generator/rtm_gen.py --check
python tools/gen_theme_tokens.py --check
python -m pytest tests/unit -q
cd api/proto && buf lint
cmake --build --preset debug && ctest --preset debug
```

CI runs all of this, plus x86_64 and ARM64 builds, the GCS build and a Trivy scan, on every pull
request and on pushes to `dev`. Work lands on `dev` and reaches `main` through a pull request.

## Documentation is part of the change

The docs in `docs/` are a living design: a document that disagrees with the code is a P1 bug, so
fix one of them in the same pull request. After any document or milestone, run the seven-lens
review (weaknesses, hidden assumptions, missing requirements, scalability, security, certification,
technical debt) and log the findings in DECISIONS.md's review log
([DOCUMENTATION_PROCESS.md](docs/08_Developer_Handbook/DOCUMENTATION_PROCESS.md)). Update
[progress.md](docs/context/phase-1/progress.md) at the end of each working session.

## Style

`.clang-format` and `.clang-tidy` for C++; Python 3.11+ with type hints; units in names, always
(`timeout_ms`, `alt_agl_m`, `roll_deg`) per [CODING_STANDARDS.md](docs/08_Developer_Handbook/CODING_STANDARDS.md).
Commit messages: a short imperative line, then the why. No co-author or generated-by trailers.

## Licence

Contributions are licensed under Apache-2.0, the same as the project.
