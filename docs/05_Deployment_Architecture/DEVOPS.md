# DEVOPS

**Version 0.1.0 · 2026-07-03 · Operating the platform across its lifecycle. CI_CD.md owns the pipeline; DEPLOYMENT.md owns install/update mechanics; this document owns the operational practice around them.**

## 1. Operating model by tier

| Tier | Operated by | Our obligation |
|---|---|---|
| Workstation | Each developer | `setup.sh` reliability, doctor tooling, fast diagnosis |
| Edge nodes | **Customer** (field techs, not SREs) | Everything must be operable by a trained technician with a runbook: guided install, one-command diagnostics, signed updates, no SSH-and-tinker expectations |
| Cloud | Us (Phase 4) | Conventional SRE practice, right-sized |

The middle row drives the whole DevOps design: UAOP's operators are aviation people, not Kubernetes people. Every operational task must have a **runbook + a CLI/GUI affordance**, and anything requiring `kubectl` fluency in the field is a product defect.

## 2. The `uaop-node` operations CLI (ships with the edge profile)

```
uaop-node status        # services, degradation ladder position, disk watermarks, chain head
uaop-node doctor        # full diagnostic sweep with plain-language findings + runbook refs
uaop-node update <bundle># signed bundle apply (wraps EDGE_ARCHITECTURE.md §6 flow)
uaop-node rollback      # previous overlay restore
uaop-node backup <dest> # consistent snapshot: PG base backup + MinIO sync + config export
uaop-node restore <src>
uaop-node export-audit <range> <dest>   # signed evidence bundle
uaop-node support-bundle                # scrubbed diagnostics archive for us
```

`doctor` is the flagship: it encodes the failure-mode tables from every architecture document as executable checks (NATS stream health, consumer lag, DB migration version, disk SMART, clock discipline, certificate expiry, firewall posture, egress canary in air-gap profile).

## 3. Backup & disaster recovery

- **What:** Postgres (WAL-archived base backups), MinIO objects, NATS stream state (recreated, not backed up — durable consumers re-derive), node config + keys (encrypted export).
- **Cadence:** edge — daily local snapshot to second disk where present + operator-scheduled external backup via `uaop-node backup`; cloud — standard managed snapshots + cross-region for the archive tier.
- **RPO/RTO honesty:** single-node edge RPO = last backup for infrastructure loss, ~zero for process crashes (PVCs survive); RTO = re-provision + restore ≤ 4 h by runbook. Customers needing better buy the dual-disk RC-2 option or accept the number — we write it down rather than imply magic.
- **Restore drills:** quarterly on the reference edge node; an untested backup is a rumor.

## 4. Runbook library (grows with the failure-mode tables)

One runbook per failure mode documented in the architecture set — the tables in EDGE_ARCHITECTURE/MICROSERVICES/etc. are the index. Format: symptom → confirm (doctor check) → impact statement (what still works — the degradation ladder tells the operator what they haven't lost) → action → escalation. Runbooks live in `docs/` and ship rendered into the GCS help panel (offline, obviously).

## 5. On-call & support posture (right-sized, phased)

Phase 1–3: no formal on-call — issue tracker + founder triage, honest response expectations. Phase 4 (paying customers): tiered support with the `support-bundle` flow (scrubbed per LOGGING.md redaction — customer flight data never leaves site without explicit consent), remote-assist only via customer-initiated outbound session, never standing inbound access to edge nodes (SECURITY.md posture).

## 6. Capacity & lifecycle management

Disk forecasting from DATABASE.md retention math surfaced in `status`/Grafana (days-until-watermark, not just percent-full); certificate and key rotation calendared and doctor-checked; upstream dependency lifecycle tracked in the RISK_REGISTER (Humble EOL, Gazebo cadence, firmware matrix) with scheduled migration spikes rather than surprise scrambles; reference-hardware fleet (lab RC-2/RC-3) mirrors the oldest supported customer configuration so "works on latest only" regressions surface in our lab first.

## 7. Configuration drift control

Edge nodes are **declaratively defined**: the applied overlay + config export fully describes a node; `doctor` diffs live state against declaration and reports drift (a hand-edited field node is a detectable condition, not a mystery). Fleet-wide config comparison arrives with the Phase 4 cloud tier.
