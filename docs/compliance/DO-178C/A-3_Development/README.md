# A-3 — Development

DO-178C Annex A Table A-3 artifacts: HLR → LLR → Architecture/Design → Source Code, with traceability at every step ("no code exists without a requirement" — COMPLIANCE.md §A.1, ARP4754A).

| Artifact | Where it lives | Notes |
|---|---|---|
| System Requirements | `docs/00_Project_Foundation/MASTER_CONTEXT.md`, `PRODUCT_REQUIREMENTS.md` §1 (platform-level) | |
| **HLR** (High-Level Requirements) | `docs/00_Project_Foundation/PRODUCT_REQUIREMENTS.md` | Every `UAOP-HLR-NNN` statement in that document *is* the HLR set — this is the source of truth, not a document to be separately re-authored here. `(T)`/`(A)` tags mark test-verifiable vs audit-verifiable requirements. |
| **LLR** (Low-Level Requirements) | Per-module, alongside source (`@req: UAOP-LLR-NNN-MM` annotations — COMPLIANCE.md §A.3 example) | LLRs land as each DAL C-equivalent module (mavlink-bridge, vehicle-manager, mission-engine transfer/verify, parameter-engine write path, remote-id, compliance-engine chain — COMPLIANCE.md §A.2) is actually implemented. None exist yet as of M1; the first LLRs are expected with the mavlink-bridge and vehicle-manager builds (M3+, IMPLEMENTATION_PLAN.md). This is deliberate — LLRs authored ahead of the design they describe drift immediately and are worse than none. |
| Architecture & Design | `docs/02_System_Architecture/`, `docs/03_Software_Architecture/` | |
| Source Code (MISRA-compliant) | `backend/`, `middleware/`, `frontend/` | MISRA C++:2023 config: `compliance/tools/misra-config/`; full coverage needs a commercial checker (Phase 2 procurement item, `compliance/tools/misra-config/README.md`) |

**Traceability convention** (enforced by `compliance/tools/rtm-generator/rtm_gen.py`, CI-gated):
```cpp
// @req: UAOP-HLR-001  MAVLink ingestion and routing
// @req: UAOP-LLR-001-03  Parse HEARTBEAT and update vehicle state
// @test: UAOP-TC-001-A  Verify HEARTBEAT parsing in SITL
```
Generated output: [`../RTM.md`](../RTM.md) — regenerate, never hand-edit.
