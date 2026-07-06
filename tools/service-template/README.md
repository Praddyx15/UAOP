# service-template — the canonical C++ service shape

The compilable reference for PROJECT_STRUCTURE.md §3's hexagonal layout. Every real engine (mission-engine, vehicle-manager, …) is scaffolded by copying this shape, not by improvising:

```
src/
├── domain/     pure C++17, zero framework includes — the MISRA/unit-test workhorse
├── app/        use cases; the audit-event emission points
├── ports/      abstract interfaces the domain/app depend on (dependency inversion)
├── adapters/   concrete impls at the edges (here: stdout; real services: NATS/gRPC/Postgres)
└── main.cpp    wiring only: config → adapters → app → run
```

The example domain is a **heartbeat/link-state monitor** (deliberately previewing vehicle-manager's UAOP-HLR-004 behavior) — pure logic, `Result<T,E>` error handling, no exceptions, no allocation in the per-sample path.

**Traceability note:** `@req:` annotations in this template use the placeholder ID `UAOP-TPL-000` and the directory is **excluded from RTM scanning** (`compliance/tools/rtm-generator`) — template code must never satisfy a real requirement's trace. When copying the template for a real service, replace placeholders with real requirement IDs.
