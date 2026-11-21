# Gap analysis

Issue 1, 2026-11-21. What existed before this lifecycle set, what exists
after it, and what a later phase that may touch code needs. The later phase
is planned and awaits the owner's go-ahead; nothing in it was started.

## 1. Before and after

| Lifecycle item | Before | After this phase |
|---|---|---|
| Classification and approval plan | DO-178C named as the process frame, with a note that DO-278A is the ground analogue; plan documents listed as "not yet authored" in `docs/compliance/DO-178C/A-2_Planning/README.md` | DO-278A proposed as the frame with reasons; one plans file |
| Stakeholder needs, ConOps | Personas and vision in `docs/00_Project_Foundation/` | ConOps and 11 needs with ids |
| System requirements | Product requirements (`UAOP-HLR`, `UAOP-NFR`) without parent links or allocation | 30 SYS with parents, allocation, method and status |
| Safety assessment | One failure condition named in prose (stale position shown as live); assurance tiers asserted | FHA with 19 failure conditions, FMEA for the command and telemetry paths, 13 safety requirements |
| Interface control | `.proto` files and design documents | Five ICDs written from the code, each listing where code and design differ |
| Software requirements | None per item; one requirement id annotated in code | 44 HLR (31 Active, as built) and 16 LLR for the gateway |
| Design description | Design intent documents | As-built HLD and one LLD |
| Problem reports | Review findings in `DECISIONS.md` | Register with 15 reports found by reading |
| Trace | Generated matrix covering one requirement id | Parent links in `.sdoc`; no tool check yet |
| Verification records, reviews, configuration index | CI only | Unchanged: planned |

## 2. What the later phase needs

| # | Step | Touches | Note |
|---|---|---|---|
| G-1 | Install StrictDoc (pinned version) and add a config with the grammar alias | Tooling, one new config file | Until then the `.sdoc` files are unchecked |
| G-2 | Decide the tag form: keep `@req:` / `@test:` or use StrictDoc source markers | Decision by the owner | See `02-standards/requirements-standard.md` section 5 |
| G-3 | Tag tests with the HLR and LLR they verify | Test files | 31 Active HLR and 16 LLR to cover |
| G-4 | Tag gateway code with its LLR | Source files | One item first |
| G-5 | A trace check in CI: unique ids, parents, every Active requirement tested, no tag naming an unknown id; generated matrix in `08-traceability/` | New tool, CI workflow | Could extend the existing matrix generator |
| G-6 | Coding-standard enforcement: confirm the MISRA configuration runs on every pull request; add linters for JavaScript and Python | CI workflow, configs | `02-standards/coding-standards.md` |
| G-7 | Work the problem reports, safety-related first (PR-001 to PR-006, PR-015) | Source and documents | Each fix names its report |
| G-8 | LLR and LLD for the vehicle link and vehicle manager as they are built (M2, M3) | New documents and code | Highest criticality; write requirements before code |
| G-9 | Verification records per baseline, coverage measurement, review records | `07-verification/`, `09-reviews/` | |
| G-10 | Agree classification and levels with an authority; validate needs with real operators | Outside the repository | Changes the plans if the answer differs |
| G-11 | Align the existing compliance documents with the classification agreed (frame, editions, level names) | Existing documents | Not done here: existing files were not edited |

## 3. Limits of this phase

Written from a reading of the repository, not from running it. No build,
test or service was run. Counts of tests and behaviour of code are as read.
The design documents were sampled, not read in full; where this set points
to one, the pointer was checked to exist but the content was not reviewed
line by line.
