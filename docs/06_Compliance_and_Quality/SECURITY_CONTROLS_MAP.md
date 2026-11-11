# SECURITY_CONTROLS_MAP

**Version 0.1.0 · 2026-11-11 · Created by ADR-0023 §6 · Maintained under IMPLEMENTATION_PLAN.md M2E.5 and at every phase gate.**

One table per external bar, mapping what a buyer will ask to what UAOP does. It exists so the same evidence serves an Indian defence conversation and a US or allied civilian one. Status values are blunt:

- **Built**: code exists and is tested in this repository.
- **Designed**: specified in a UAOP document, not built yet.
- **Gap**: neither; needs a decision or design.

Nothing here is a certification claim. UAOP has not been assessed by any lab or agency.

## 1. OMB M-26-02, Appendix B §2 (US civilian federal UAS procurement)

Source: OMB Memorandum M-26-02, *Ensuring Government Use of Secure Unmanned Aircraft Systems and Supporting United States Producers*, 21 Nov 2025 ([PDF](https://www.whitehouse.gov/wp-content/uploads/2025/11/M-26-02-Ensuring-Government-Use-of-Secure-Unmanned-Aircraft-Systems-and-Supporting-United-States-Producers.pdf)), read 2026-11-11. It applies to civilian agencies and federally funded grants, not DoD. Agencies "must document how they will ensure" these capabilities; most items say "should". The memo applies them "regardless of where the UAS was manufactured or assembled".

| Item | Requirement (paraphrased) | UAOP control | Status |
|---|---|---|---|
| A.1 | Remote access to UAS ground control stations uses appropriate authentication, including MFA per NIST SP 800-63 | api-gateway v0: JWT with six-role RBAC, tokens ≤ 60 min, Redis denylist (built, 11 tests). Local identity provider with Argon2id and **optional** TOTP (SECURITY.md §3) | **Built** (tokens, RBAC) / **Gap** (MFA is optional). Proposed: TOTP mandatory for any non-loopback session, decided before M3.4 exposes command endpoints |
| A.2 | Where availability impact is moderate or high, manage the UAS under an IT asset framework (NIST SP 1800-5) | Node identity keypair at provisioning; vehicle registry (vehicle-manager, M2.5) | **Designed** |
| B.1 | Software and firmware updates only from the manufacturer or a trusted third party | Update bundles, plugins, models and export bundles Cosign-signed against a trust store the customer controls (SECURITY.md §3) | **Designed** |
| B.2 | IT used to install or download updates is isolated from enterprise systems | `edge-airgap` profile, default-deny egress; signed-bundle-only USB import (SECURITY.md §2, §6) | **Designed** |
| B.3 | Where integrity impact is moderate or high, file-integrity check and test updates before missions | Signature verification on import; SITL matrix before release. Proposed for M2E.3: the evidence bundle records the autopilot's reported firmware version and git hash (MAVLink `AUTOPILOT_VERSION`), so a changed firmware is visible flight to flight | **Designed** / proposed |
| C.1 | Mission data encrypted at rest and in transit where practicable | LUKS on edge volumes; TLS 1.3 on all non-loopback transport (SECURITY.md §4–5). The development compose stack runs without TLS | **Designed** |
| C.2 | Sensitive data secured with approved, validated cryptographic modules | All crypto behind one library seam; FIPS-validated build variant planned for Phase 4 (SECURITY.md §4) | **Designed** (Phase 4) |
| C.3 | Agency can opt out of any upload or transmission not required by law, and choose where data is stored | Air-gap is permanent (UAOP-NFR-001); FlightMD hand-back off unless `UAOP_FLIGHTMD__URL` is set, redirects refused, tokens from file (ADR-0022, R7/F47) | **Built** (sidecar) / **Designed** (platform) |
| C.4 | Consider remote security capabilities (remote wipe or lock) triggerable without the manufacturer | None specified | **Gap**. Needs a decision: an operator-held key-revocation path for edge-node volumes is the likely shape |
| C.5 | Erase moderate/high-confidentiality data collected by the UAS after each mission | Conflicts with an append-only audit chain if applied naively | **Gap**. Proposed: payload data (video, imagery, raw logs) purgeable per retention policy, while the audit chain keeps hashes so the record of what existed survives deletion |
| C.6 | Where confidentiality impact is high, technical controls disable storage and transmission to non-approved systems | Default-deny egress, CI canary asserting zero external sockets during the test suite (SECURITY.md §6) | **Designed** |

## 2. Indian Army AASHVAST lab test areas

Source: reporting on the AASHVAST lab (Assessment and Analysis of Electronic Systems Hardware for Vulnerabilities and Security Threats), opened about 14 Aug 2026 for DG EME ([The Week, 9 Sep 2026](https://www.theweek.in/news/defence/2026/09/09/whats-hiding-inside-indias-military-drones-indian-armys-aashvast-to-eliminate-chinese-parts-security-risks.html)). No primary DG EME document was found. The lab tests **drone hardware and on-board firmware**; no source puts ground-control or ops software in scope (R-18). UAOP's role is therefore twofold: hold its own ground software to the same bar, and record evidence about the aircraft it operates or observes.

| Test area | UAOP's own software | Evidence UAOP records about the aircraft | Status |
|---|---|---|---|
| Hidden passwords, embedded keys | No secrets in code or images; tokens from files, never env dumps (R7/F47); Trivy scan in the PR pipeline | — (aircraft firmware is out of UAOP's reach) | **Built** (CI scan) |
| Remote-access tools, beaconing | No phone-home; default-deny egress; licensing never grounds an aircraft (BUSINESS_MODEL.md §3) | — | **Designed** |
| Accepts only signed updates | Cosign-signed bundles (§1 B.1) | MAVLink 2 signing state per link, surfaced in the UI (SECURITY.md §2, §9); firmware version per flight (§1 B.3 proposal) | **Designed** |
| Location-based controls (geofencing) | — | Geofence upload and breach events in the audit chain (UAOP-HLR-011, M4.3) | **Designed** |
| GPS spoofing and jamming response | — | Jamming/spoofing telemetry surfaced as first-class (SECURITY.md §2); FlightMD GPS analyser findings in the evidence bundle (UAOP-HLR-031, -044) | **Designed** / partly **Built** (sidecar) |
| Chinese-origin components | SBOM per artifact (SECURITY.md §2); licence allow-list gate in CI (`tools/ci/check_licenses.py`) | — | **Built** (licence gate) / **Designed** (SBOM). Origin of each dependency is not yet recorded; add an origin column to the SBOM when it is built |

## 3. Not mapped yet

DAP 2020/2026 software clauses, STQC/ETDC software testing, CERT-In empanelled audits and NCIIPC requirements: the 2026-11-11 research found nothing verifiable about how they apply to ground or ops software. Revisit when any is published (R-18). The DCMA Blue UAS framework's treatment of GCS software and foreign software vendors is likewise unverified (COMPETITOR_ANALYSIS.md §5).
