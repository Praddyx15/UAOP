# SECURITY

**Version 0.1.0 · 2026-07-03 · Threat-model-driven; hardening deepens by phase, but nothing here is retrofittable-by-design (identity, audit, input hardening start Phase 1).**

## 1. Assets, in priority order

1. **Command path integrity** — nothing unauthorized may reach a vehicle.
2. **Audit chain integrity** — evidence must be tamper-evident (compliance-engine hash chain).
3. **Flight data confidentiality** — positions/missions are operationally sensitive (defense: classified-adjacent).
4. **Platform availability** — DoS of the edge node during flight is a safety event.
5. **Supply chain** — we ship signed compositions of open-source; provenance is the product's credibility.

## 2. Threat model summary (STRIDE-organized, key actors)

| Actor | Vector | Primary controls |
|---|---|---|
| RF-capable adversary | MAVLink injection/replay on radio links; GPS jamming/spoofing | MAVLink 2 signing (required on WAN/LTE, recommended RF); jamming/spoofing *telemetry surfaced as first-class data* (rf category) — detection is an operator instrument, not a hidden log line |
| Network intruder (operator LAN) | Gateway abuse, credential theft, lateral movement | TLS everywhere non-loopback; JWT + RBAC at gateway **and** re-checked at vehicle-manager; default-deny host firewall; NATS/DB ports never exposed off-node |
| Malicious/compromised plugin | Sandbox escape, capability abuse | PLUGIN_SYSTEM.md §3: signing, capability tokens, process isolation, no `uaop.cmd.>` for any plugin, quarantine |
| Insider / careless operator | Unauthorized commands, parameter sabotage, audit evasion | RBAC six-role model + per-vehicle ControlSession authority (ADR-0017); risk-classified writes; fail-closed audit at durable event persistence (ADR-0015); every mutation carries actor identity |
| Supply-chain attacker | Poisoned dependency/image/model | Pinned + hashed dependencies, SBOM per artifact, Cosign-signed images and update bundles, signed model registry (AI_ENGINE.md §5) |
| Physical access to edge node | Disk theft, USB attacks | LUKS at rest; signed-bundle-only USB import (no autorun side door); boot integrity on capable hardware (Phase 4) |

## 3. Identity & access

- **Humans:** local identity provider Phase 1–3 (Argon2id-hashed credentials, TOTP optional); OIDC federation Phase 4. Six roles (PLATFORM_ADMIN, ORG_ADMIN, FLEET_MANAGER, PILOT, OBSERVER, MAINTENANCE); role→permission matrix versioned in `docs/context/` and enforced twice (gateway route policy + service-level checks on consequential actions). Token discipline (*added by review R1/F5*): access tokens ≤ 60 min with refresh rotation; Redis-backed denylist for immediate revocation (logout, compromise, role change) — interim until OIDC session management arrives in Phase 4.
- **Services:** per-service NATS credentials with subject-level allow-lists (this is what makes ADR-0012's "ai-engine cannot publish commands" *structural*); per-service DB roles (append-only for audit writer); gRPC intra-node mTLS in Phase 4 (Phase 1–3: loopback/netns isolation + network policies).
- **Nodes & artifacts:** node identity keypair generated at provisioning; update bundles, plugins, models, export bundles all Cosign-signed against a trust store the customer controls.

## 4. Cryptography posture

TLS 1.3 on all non-loopback transport; SHA-256 audit chaining; Argon2id for credentials; Ed25519 for artifact signing. **Phase 4 hardening:** FIPS-validated module (BoringSSL FIPS mode) as a build variant, HSM/SoftHSM2 interface for node keys, mTLS mesh internally, zero-trust posture review. Designed-not-bolted: all crypto goes through one internal library seam so the FIPS variant is a swap, not a hunt.

## 5. Data protection

At rest: LUKS (edge PVCs), encrypted k3s secrets, per-org schema isolation in cloud (CLOUD_ARCHITECTURE.md §4). In transit: §4. In use: no secrets in logs (LOGGING.md redaction rules), no telemetry in error reports without scrubbing. Export control: signed export bundles carry a manifest of contents; audit records who exported what (TELEMETRY_ENGINE.md §6).

## 6. Air-gap as a security property

The `edge-airgap` profile is default-deny egress at the host firewall, verified by CI (a canary asserts zero external sockets during the full test suite). This converts an entire class of exfiltration/beaconing threats into a physically checkable property — a core sales argument for defense (VISION.md moat).

## 7. Secure development

- Static analysis gates from commit one (Cppcheck + clang-tidy + MISRA ruleset; Bandit/ruff for Python; CodeQL in CI Phase 2+).
- Fuzzing: mavlink-bridge parser (structure-aware) and gateway input surfaces, corpus grown in CI (MAVLINK_INTEGRATION.md §6).
- Dependency scanning (Trivy) + license audit per release; images minimal/distroless where practical.
- Security review is a phase-gate item (VALIDATION.md layer 5), and the RISK_REGISTER carries security items with owners.

## 8. Incident posture (right-sized for stage)

Phase 1–3 (pre-customer): security defects are P1 bugs with a disclosure note in release notes. Phase 4 (customers): documented IR runbook, customer notification policy, signed advisories, and the audit chain doubles as forensic timeline. We do not promise a 24/7 SOC we don't have; we promise honest artifacts that make a customer's own IR effective.

## 9. Known accepted risks (stated, not hidden)

- MAVLink over plain RF without signing (customer hardware constraints) — mitigated by monitoring, not eliminated; flagged per-link in UI.
- ROS 2/DDS security off by default (ROS2_INTEGRATION.md §6) — contained at the bridge, honestly labeled.
- Node.js gateway runtime enlarges supply-chain surface (ADR-0004 reservation) — revisit at Phase 4 hardening.
