# LOGGING

**Version 0.1.0 · 2026-07-03 · Application/diagnostic logging. Flight telemetry is data (TELEMETRY_ENGINE.md); audit is evidence (COMPLIANCE.md §B.3); this document is about the third stream: what engineers and `doctor` read. Confusing the three is a design error this page exists to prevent.**

## 1. The three streams, disambiguated

| Stream | Contents | Guarantees | Consumer |
|---|---|---|---|
| Audit | Who did what to which vehicle | Append-only, hash-chained, permanent, fail-closed | Auditors, compliance-engine |
| Telemetry/flight logs | What the aircraft did | Gap-accounted, hash-at-capture | Operators, analysts, ai-engine |
| **Diagnostic logs (this doc)** | What the software did and why | Best-effort, rotated, redacted | Engineers, doctor, support |

Rule of thumb enforced in review: if losing the record would upset an auditor → audit event; a pilot/analyst → telemetry; only an engineer → diagnostic log. An event may legitimately produce two of these (a failsafe is audit + telemetry + probably a log line) — but each via its own channel, never "grep the logs" as a compliance answer.

## 2. Format & conventions

Structured JSON lines to stdout (container-native), one schema platform-wide:

```json
{"ts":"2026-07-03T10:15:04.221Z","lvl":"WARN","svc":"mavlink-bridge","vehicle":"veh-01",
 "corr":"01J...","event":"link.rssi_degraded","msg":"RSSI below threshold","rssi":-92,
 "link":"serial:/dev/ttyUSB0"}
```

- Levels: ERROR (operator-visible problem or invariant violation), WARN (degradation, retries, shed events), INFO (lifecycle, state transitions), DEBUG (off in production profiles, per-service toggle at runtime — audit-logged toggle, since verbose logs can leak operational detail).
- `event` is a stable snake_case identifier (greppable, alertable); `msg` is for humans; **variables go in fields, not interpolated into `msg`** — this is what makes 3 a.m. debugging and log-based alerting possible.
- `corr` propagates the correlation ID across service hops (one command traceable gateway→vehicle-manager→bridge in three log stores).
- Hot-path discipline: no logging inside the RT telemetry path except counters flushed periodically (ADR-0013's no-alloc rule applies to log formatting too — a log call that mallocs on the 1 kHz path is a defect).

## 3. Redaction (SECURITY.md §5 enforcement point)

Never loggable: credentials/tokens/keys (structural: the config/secret types implement redacted `format()`); full parameter dumps (log names + counts, values only at DEBUG with explicit toggle); precise positions at INFO in `support-bundle` exports (scrubber quantizes unless customer consents — flight data confidentiality, SECURITY.md asset 3). The `support-bundle` scrubber is tested, not aspirational: golden tests feed synthetic secrets/positions through and assert absence.

## 4. Collection & retention

Edge: k3s container logs → local aggregation (Loki, single-binary mode — fits the resource budget) → 15-day retention, size-capped, oldest-evicted (diagnostic logs are the *first* thing sacrificed under disk pressure, per the DATA_FLOW.md priority: they are the only stream that's allowed to die silently... with a counter). Workstation: plain compose logs. Cloud: managed aggregation, 30–90 d by tier. `uaop-node support-bundle` packages the relevant window scrubbed.

## 5. What gets logged, by policy (excerpts)

- Every service: startup config fingerprint (hash, not contents), version, contract versions; readiness transitions; consumer lag warnings; DLQ deliveries (EVENT_FLOW.md §7 — a DLQ hit is always ≥ WARN).
- mavlink-bridge: link lifecycle, per-link quality transitions, malformed-frame counters (periodic summary, never per-frame), signing failures.
- gateway: authn/z failures (with identity, without credentials), rate-limit hits, version-handshake refusals.
- Never: per-telemetry-message logging at any level (that's what metrics and the telemetry store are for — a log line per sample is how edge disks die).

## 6. Relationship to observability

Logs are for *why*, metrics for *how much*, traces (Phase 2+, OBSERVABILITY.md) for *where*. Alerting keys on metrics and specific `event` identifiers — never on log-volume heuristics. If a condition matters enough to page about, it earns a metric or a structured event; log-scraping alerts are tech debt by definition here.
