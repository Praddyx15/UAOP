# OBSERVABILITY

**Version 0.1.0 · 2026-07-03 · Prometheus + Grafana baseline; the platform must be diagnosable by a field technician and forensically explicable to us. Logs: LOGGING.md.**

## 1. Golden signals, UAOP-specific

Generic RED/USE metrics exist per service; the *platform-defining* signals are:

| Signal | Source | Why it's golden |
|---|---|---|
| Vehicle link quality/loss rate per link | mavlink-bridge | The operator's lifeline |
| Hot-path latency segments (bridge→NATS→gateway→WS) | instrumented histogram per DATA_FLOW.md §3 | UAOP-NFR-002 lives here, continuously, not just at gates |
| JetStream consumer lag per durable consumer | NATS exporter | Early warning for every downstream failure |
| Telemetry gap rate + shed events | telemetry-engine, node supervisor | The honesty metrics — nonzero demands explanation |
| Audit event persistence latency (JetStream) + chain append lag | compliance-engine, NATS exporter | Event persistence gates command admission (ADR-0015); chain lag raises CAUTION before it becomes an evidence problem |
| Degradation ladder position | node supervisor | The single number a field tech reads first |
| Disk days-until-watermark | node exporter + retention math | The most likely field failure, forecast not just measured |
| DLQ delivery count | NATS | Always-investigate signal (EVENT_FLOW.md §7) |

## 2. Stack per profile

Edge: Prometheus (local, 15 d retention) + Grafana + node exporter + NATS/Postgres exporters — inside the resource budget (EDGE_ARCHITECTURE.md §3 infrastructure tier); dashboards ship pre-provisioned (see §4). Workstation: same, optional. Cloud: managed Prometheus-compatible + long-horizon fleet dashboards over synced ops metrics (ops metrics sync in the lowest-priority data class).

## 3. Alerting philosophy

Three severities, ruthlessly curated:

- **PAGE-equivalent (edge: GCS master-caution integration + doctor surfacing):** vehicle link loss (already a UX event — alerting and operator UX share definitions), audit write failure, degradation ladder ≥ level 4, disk ≥ 90%, service crash-loop.
- **INVESTIGATE (daily review):** consumer lag trends, gap-rate nonzero, DLQ hits, latency p95 regression, cert expiry < 30 d.
- **RECORD (dashboards only):** everything else.

Rule: every alert has a runbook link (DEVOPS.md §4) or it doesn't ship; alert fatigue in a safety-adjacent product is a hazard, so the alert set is reviewed at each phase gate and anything with a >50% ignore rate is demoted or fixed.

## 4. Dashboards (pre-provisioned, versioned in `infrastructure/monitoring/`)

1. **Node overview** — ladder position, service health grid, link map, disk forecast (the field tech page; mirrors `uaop-node status`).
2. **Telemetry pipeline** — rates by category, ingest batch performance, gaps, lag, hot-path histogram.
3. **Vehicle links** — per-link RSSI/quality/loss, signing status, reconnect counts.
4. **Compliance health** — chain head age, audit rates by domain, export history.
5. **Per-service drill-downs** — RED + service-specific (bridge frame counters, gateway WS fan-out queues, ai-engine job queue).

## 5. Tracing (Phase 2+)

OpenTelemetry on the **command path only** (gateway→vehicle-manager→bridge→ACK): commands are where multi-hop latency mysteries and correctness questions concentrate, and the volume is low enough for 100% sampling. Telemetry-path tracing is refused by design — histograms answer the performance question there without per-sample overhead on the 1 kHz path. Trace IDs join the existing correlation-ID scheme (LOGGING.md §2) rather than inventing a second causality system.

## 6. Metrics discipline

Prometheus naming conventions (`uaop_<service>_<subject>_<unit>`); cardinality budgeted — labels are vehicle_id, category, link (bounded sets); **never** per-message or per-user labels; a new metric with unbounded label values fails review. Metrics are contracts too: dashboards and alerts pin metric names, so renames follow the deprecation discipline like any API.

## 7. Forensics: the "what happened at 14:32" workflow

Design target — any incident reconstructable from: degradation/gap events (what the platform shed and when) + audit chain (what humans and services did) + telemetry store (what the aircraft did) + correlated diagnostic logs (why code paths chose what they chose) + command traces (where time went). VALIDATION.md's fault-injection suite includes a forensics drill: inject a compound failure, then reconstruct it from stored data alone — if reconstruction needs a debugger or tribal memory, observability has a gap to fix before the gate.
