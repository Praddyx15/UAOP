# flight-log-engine · analysis sidecar

Post-flight analysis for flight-log-engine, done by FlightMD's `flightmd-core`
package (MIT, ADR-0022). Python 3.11 + FastAPI, beside the C++ capture/storage
service that M5.2 builds, the same split compliance-engine uses for its SORA
sidecar (Review R1/F8).

What it does:

- `POST /v1/analyses` `{log_name, vehicle_id?, airframe_label?}` runs flightmd-core on a
  log from the log library (`UAOP_FLIGHT_LOG__LOG_DIR`), drops ascent-profile findings
  (R4/F26) and stores the report.
- `GET /v1/flight-reports[?vehicle_id=]` and `GET /v1/flight-reports/{report_id}`:
  the flights, and score + findings + parameter sheet for one of them. Same fields as
  `ListFlightReports` / `GetFlightReport` in `api/proto/uaop/gateway/v1/log.proto`.
  The GCS post-mission panel reads these.
- Consumes `uaop.event.v1.param.written` (durable `flight-log-engine-param-written`).
  When a write carries a FlightMD recommendation (`events/v1/param.proto`), it calls
  FlightMD's `POST /airframe/{label}/changes` with `applied_by: "uaop"` so FlightMD's
  next report of that airframe can verify the change (tune, fly, verify).
- `/healthz`, `/readyz` (503 without flightmd-core), `/metrics`.

No AIEnhancer is ever passed: the analysis makes no network call (ADR-0018). The
FlightMD hand-back is off unless `UAOP_FLIGHTMD__URL` is set, and should point at a
self-hosted FlightMD on the node or LAN, not the public instance.

## Config

| Variable | |
|---|---|
| `UAOP_FLIGHT_LOG__DATA_DIR` | required; reports and the forwarded-event ledger |
| `UAOP_FLIGHT_LOG__LOG_DIR` | required; the log library analyses read from |
| `UAOP_FLIGHTMD__URL` | FlightMD base URL; empty = hand-back off |
| `UAOP_FLIGHTMD__TOKENS_FILE` | JSON `{"<airframe label>": "<X-Airframe-Token>"}` |
| `UAOP_FLIGHTMD__TIMEOUT_S` | default 10 |
| `UAOP_NATS_URL` | empty = no param.written consumer |

## Run and test

```bash
cd backend/services/flight-log-engine/analysis-sidecar
pip install -r requirements-dev.txt
pytest
uvicorn flight_log_analysis.app:app --host 127.0.0.1 --port 8095
```

`flightmd-core>=1.1,<2` comes from PyPI once 1.1 is published. Until then install it
from the FlightMD repo at a pinned commit, then the rest:

```bash
pip install "flightmd-core @ git+https://github.com/Praddyx15/FlightMD@<commit>"
pip install -r requirements-dev.txt
```

The NATS consumer imports the Python protobuf modules buf generates from `api/proto`
(`buf generate` → `gen/python`, put it on `PYTHONPATH`).

## Not yet

- Reports live as JSON files under the data dir; Postgres/MinIO with the rest of the
  log library is M5.2.
- Nothing publishes `param.written` with a recommendation yet: parameter-engine is
  M4.4. Its side of the contract is `parameter-engine/include/uaop/parameter_engine/applied_recommendation.h`.
- api-gateway has no route to these endpoints yet; the GCS reads them directly on
  loopback in development (`UAOP_FLIGHT_LOG_URL`).
