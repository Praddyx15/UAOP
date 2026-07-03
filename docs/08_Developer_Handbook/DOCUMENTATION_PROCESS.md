# DOCUMENTATION_PROCESS

**Version 0.1.0 · 2026-07-04 · How the living design system works: structure, lifecycle, the mandatory critical review, and the context-budget discipline for AI-assisted sessions.**

## 1. The numbered design system

Documentation lives in nine numbered areas, built and evolved **incrementally** — one area (or less) per working session, never the whole set at once:

| Area | Contents | Change frequency |
|---|---|---|
| `00_Project_Foundation` | Constitution, vision/mission, requirements, ADRs, roadmap, risks, market | Low; every change is significant |
| `01_Product_Architecture` | Engines, plugin system, SDK, digital twin — *what the product does* | Per-phase |
| `02_System_Architecture` | System/hardware/edge/cloud, flows, ecosystem integrations | Per-phase |
| `03_Software_Architecture` | Software patterns, service catalog, APIs, data stores | With implementation |
| `04_UI_Architecture` | UI/UX/style for the Qt GCS | With GCS milestones |
| `05_Deployment_Architecture` | Deploy, DevOps, CI/CD, logging, observability | With ops reality |
| `06_Compliance_and_Quality` | Compliance, security, validation, testing | Gate-driven |
| `07_Implementation_Guides` | The executable plan + project blueprint | Every milestone |
| `08_Developer_Handbook` | Standards, repo rules, this process | Rarely |

Rules: `MASTER_CONTEXT.md` wins conflicts; resolution only via ADR. Cross-references use relative links (CI link-check keeps them alive). A document that disagrees with the code is a **P1 defect** — fix the doc or fix the code in the same PR.

## 2. Document lifecycle

`DRAFT` → `REVIEWED` (critical review done, findings dispositioned) → `BASELINED` (version-stamped at a phase gate) → thereafter amended only with a version bump + changelog line at the top. Generated artifacts (RTM, API reference) are never hand-edited — regenerate or fix the source.

## 3. Context-budget discipline (AI-assisted authoring)

The context window is a real constraint; the structure exists to respect it:

- **One area per session.** Load `MASTER_CONTEXT.md` + `DECISIONS.md` + the target area only; other areas are consulted by name, not re-read wholesale.
- **DECISIONS.md is the session-to-session memory.** Any cross-area consequence discovered mid-session is recorded there immediately, even if the affected document is edited in a later session (tag it `PENDING-EDIT` with the target file).
- **End every session** by updating `docs/context/phase-N/progress.md` — the next session starts by reading it, not by re-deriving state.

## 4. The mandatory critical review (after every document or milestone)

Never assume the first design is the best design. On finishing any document (or code milestone), run the seven-lens review:

1. **Weaknesses** — where does this design bend or break under load, failure, or misuse?
2. **Hidden assumptions** — what does it silently take for granted (one operator? one node? clocks? connectivity?)
3. **Missing requirements** — what did writing it reveal that PRODUCT_REQUIREMENTS doesn't cover?
4. **Future scalability issues** — what stops working at 10× vehicles/data/users?
5. **Security risks** — new attack surface, trust assumptions, data exposure?
6. **Certification concerns** — does anything overclaim, underclaim, or complicate the DO-178C/DO-278A argument?
7. **Technical debt introduced** — what shortcut was taken, and where is it recorded?

**Output discipline:** findings go into the **Review Log in [DECISIONS.md](../00_Project_Foundation/DECISIONS.md)** (numbered Rn/Fm), each with a disposition — new ADR, requirement added, OQ opened, wording corrected, or *accepted with rationale*. Affected documents are edited in the same pass; nothing is "noted for later" without a `PENDING-EDIT` tag. Review R1 (the full-baseline review) is the worked example: twelve findings, three ADRs (0015–0017), two new OQs, two new NFRs, nine documents corrected.

## 5. Review empiricism (keeping the process honest)

A review that never finds anything is broken. Expect early documents to yield 2–5 findings each; if three consecutive reviews find zero, the reviewer (human or AI) is pattern-matching, not reviewing — change the lens order, review against a different persona (auditor, field tech, attacker), or diff the document against the failure-mode tables of its dependencies. Findings are cheap before code exists and exponentially costlier after; the review is the highest-leverage engineering activity in this project's current stage.

## 6. Consistency tooling (grows with the repo)

CI doc jobs (from M0): relative-link checker; ADR-reference checker (every "ADR-nnnn" string resolves); orphan check (every doc reachable from README); frozen-directory guard (PROJECT_STRUCTURE.md §2). Aspirational (Phase 2+): terminology linter against a glossary; drift check that MICROSERVICES.md's dependency declarations match the real import graph (shared with CI_CD.md §5).
