# Requirements standard

UAOP-RS, issue 1, 2026-11-21. How requirements are written and kept at every
level, in StrictDoc. Reviews cite the rule ids (RS-nn). No rule is checked by
a tool in this phase; all are review items until the trace check exists
(`docs/lifecycle/00-project-record/gap-analysis.md`).

## 1. Levels

| Level | Id form | Document | States | Parent | Verified by |
|---|---|---|---|---|---|
| Stakeholder need | `STK-nnn` | `03-system/stakeholder-needs.sdoc` | What a stakeholder needs | None; SOURCE names the ConOps section | Validation with the stakeholder |
| Safety requirement | `SAF-nnn` | `04-safety/safety-requirements.sdoc` | What must hold to keep a failure condition at its class | None; SOURCE names the failure condition | Through its SYS children; analysis |
| System requirement | `SYS-nnn` | `03-system/system-requirements.sdoc` | What the system does, allocated to items | One or more STK or SAF | Tests of its HLR children; system tests; analysis |
| Software high-level requirement | `HLR-<AREA>-nnn` | `05-requirements/hlr.sdoc` | Behaviour of one software item at its interfaces | Exactly one SYS | Requirements-based tests |
| Software low-level requirement | `LLR-<ITEM>-nnn` | `05-requirements/llr-<item>.sdoc` | Detail from which code is written: inputs, outputs, limits, errors | One or more HLR | Tests; review only with a stated reason |

Relation to the ids already in the repository: `UAOP-HLR-nnn` and
`UAOP-NFR-nnn` in `docs/00_Project_Foundation/PRODUCT_REQUIREMENTS.md` are
product requirements. They sit at system level here, and each SYS that
restates one names it in COMMENT. They are not renumbered and not copied.
The `HLR-<AREA>-nnn` ids of this set are software requirements of one item
and are a different series.

Areas: `GW` API gateway, `COM` common library, `CON` contracts and bus
provisioning, `STO` storage schema, `FLA` flight-log analysis sidecar, `GCS`
desktop station, `LNK` vehicle link (planned), `VM` vehicle manager
(planned), `TOOL` build and check tools.

## 2. Rules

| Rule | Text |
|---|---|
| RS-01 | Every requirement is a `[REQUIREMENT]` node of `02-standards/requirements.sgra`, fields in grammar order, choice values from the grammar |
| RS-02 | UIDs are unique across all documents and never reused. A dropped requirement keeps its UID with `STATUS: Deleted` |
| RS-03 | Parent levels: SYS to STK or SAF; HLR to SYS; LLR to HLR. STK and SAF have a SOURCE |
| RS-04 | SYS, HLR and LLR state a VERIFICATION method (Test, Analysis, Inspection, Demonstration) |
| RS-05 | One requirement, one testable statement. Numbers have units and limits |
| RS-06 | Say what, not how, unless the how is the requirement (a format, a protocol) |
| RS-07 | State failure behaviour: what is refused, what is recorded, what the caller sees |
| RS-08 | ALLOCATION names items of `03-system/system-architecture.md`. HLR and LLR name software items only |
| RS-09 | STATUS: `Active` when built and in force; `Proposed` when needed and not yet built; `Deleted` when dropped |
| RS-10 | A requirement the parent does not ask for is marked `DERIVED: Yes` with a RATIONALE and goes to the safety assessment |
| RS-11 | An `Active` requirement is written from the code as built. Where the code does less than its parent asks, the gap is a problem report, not a softened requirement |
| RS-12 | No requirement contradicts an interface control document or a contract in `api/proto/`. Where the contract is the requirement, the HLR points at it |
| RS-13 | (Planned) Every active HLR and LLR is named by at least one test, and every active LLR by a code tag |

## 3. Fields

`UID`, `STATUS`, `TITLE` (short name), `STATEMENT` ("shall" at STK, SAF and
SYS level; present tense for as-built HLR and LLR), `RATIONALE`, `SOURCE`,
`VERIFICATION`, `VERIFICATION_NOTE`, `ALLOCATION`, `DERIVED`, `COMMENT`
(partial compliance, the product requirement restated, problem reports),
`RELATIONS` (`- TYPE: Parent` / `  VALUE: <UID>`).

## 4. Format

StrictDoc `.sdoc` files: `[DOCUMENT]` header with TITLE, UID, VERSION, DATE
and `MARKUP: Markdown`; `[GRAMMAR]` with `IMPORT_FROM_FILE` giving the
relative path to `requirements.sgra` (to confirm against the StrictDoc
version chosen; an alias in a StrictDoc config may replace it); `[TEXT]` for
prose; `[[SECTION]]` and `[[/SECTION]]` for grouping. Multi-line values go
between `>>>` and `<<<`. Files are UTF-8 with LF line endings.

## 5. Tags in code and tests (planned)

The repository already tags code with `@req:` and `@test:` comments read by
`compliance/tools/rtm-generator/rtm_gen.py`. Whether the HLR and LLR of this
set reuse that form or StrictDoc's own source markers is an open decision for
the owner; it is listed in the gap analysis. No tag was added in this phase.
