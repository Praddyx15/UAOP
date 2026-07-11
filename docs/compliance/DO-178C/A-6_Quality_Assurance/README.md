# A-6 — Quality Assurance

DO-178C Annex A Table A-6 artifacts: process compliance and independence — "trust the process, not assumptions."

| Artifact | Where it lives | Notes |
|---|---|---|
| SQA Reports | Not yet authored | Substance today: CI's guard jobs (`tools/ci/check_frozen_dirs.py`, `check_doc_links.py`, `check_licenses.py`, `rtm_gen.py --check`) are automated, always-on process-compliance checks — a stronger continuous-QA posture than a periodic manual SQA report, but not a substitute for one at certification time |
| Audit Logs | `AUDIT` NATS JetStream stream (ADR-0015) + PostgreSQL hash-chain (COMPLIANCE.md §B.3) | This is the **product's** audit log (operational evidence — commands, state changes), distinct from a DO-178C **process** audit log (evidence the development process itself was followed). The latter doesn't exist yet as a discrete artifact. |
| Non-conformance Reports | GitHub Issues (informal) | No formal non-conformance-report template or register yet |

**Independence note (DO-178C intent):** QA activities are meant to be independent of the developer. Sixty Motion Aerospace is currently a single-founder effort — this is a documented, honest gap (RISK_REGISTER.md), not something this folder structure can resolve on its own.
