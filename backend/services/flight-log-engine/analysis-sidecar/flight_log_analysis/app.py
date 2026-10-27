"""HTTP surface of the analysis sidecar (FastAPI, ADR-0022).

Routes mirror log.proto's ListFlightReports / GetFlightReport, with the same field
names. Internal only: the node firewall keeps it behind api-gateway like every other
service (MICROSERVICES.md), and it binds to loopback by default.

    uvicorn flight_log_analysis.app:app --host 127.0.0.1 --port 8095
"""

import asyncio
import logging
from contextlib import asynccontextmanager
from typing import Any

from fastapi import FastAPI, HTTPException
from fastapi.responses import PlainTextResponse
from pydantic import BaseModel, Field

from . import analysis, config, nats_consumer
from .flightmd_client import FlightmdClient
from .store import ReportStore

log = logging.getLogger(__name__)


class AnalyseRequest(BaseModel):
    # a file name inside UAOP_FLIGHT_LOG__LOG_DIR, where capture (M5.2) puts downloaded logs
    log_name: str = Field(min_length=1, max_length=255)
    vehicle_id: str = Field(default="", max_length=64)
    airframe_label: str = Field(default="", max_length=64)


def create_app(cfg: config.Config | None = None) -> FastAPI:
    cfg = cfg or config.load()
    store = ReportStore(cfg.data_dir)
    client = FlightmdClient(cfg.flightmd_url, cfg.flightmd_tokens, cfg.flightmd_timeout_s)
    analyses: dict[str, int] = {}
    handbacks: dict[str, int] = {}

    @asynccontextmanager
    async def lifespan(app: FastAPI):
        task = None
        if cfg.nats_url:
            task = asyncio.create_task(nats_consumer.run(cfg.nats_url, client, store, handbacks))
        yield
        if task is not None:
            task.cancel()

    app = FastAPI(title="flight-log-engine analysis sidecar", lifespan=lifespan)
    app.state.store = store

    @app.get("/healthz")
    def healthz() -> dict[str, str]:
        return {"status": "ok"}

    @app.get("/readyz")
    def readyz() -> dict[str, Any]:
        try:
            import flightmd_core
        except ImportError:
            raise HTTPException(503, "flightmd-core is not installed") from None
        return {"status": "ready", "flightmd_core": getattr(flightmd_core, "__version__", "unknown"),
                "flightmd_handback": "on" if client.enabled else "off"}

    @app.get("/metrics", response_class=PlainTextResponse)
    def metrics() -> str:
        lines = ["# TYPE uaop_fle_analyses_total counter"]
        lines += [f'uaop_fle_analyses_total{{result="{k}"}} {v}' for k, v in sorted(analyses.items())]
        lines.append("# TYPE uaop_fle_flightmd_handback_total counter")
        lines += [f'uaop_fle_flightmd_handback_total{{result="{k}"}} {v}' for k, v in sorted(handbacks.items())]
        return "\n".join(lines) + "\n"

    @app.post("/v1/analyses", status_code=201)
    async def create_analysis(body: AnalyseRequest) -> dict[str, Any]:
        path = (cfg.log_dir / body.log_name).resolve()
        # no ../ or absolute paths out of the log library
        if path.parent != cfg.log_dir or not path.is_file():
            raise HTTPException(404, "no such log in the log library")
        try:
            report = await analysis.analyse(path)
        except ValueError as exc:
            analyses["rejected"] = analyses.get("rejected", 0) + 1
            raise HTTPException(422, str(exc)) from None
        view = analysis.uaop_view(report, vehicle_id=body.vehicle_id, airframe_label=body.airframe_label)
        store.save(view, report)
        analyses["ok"] = analyses.get("ok", 0) + 1
        return view

    @app.get("/v1/flight-reports")
    def list_reports(vehicle_id: str = "") -> dict[str, Any]:
        # TODO(M5.2) page_size/page_token once the library is in Postgres
        return {"reports": store.list(vehicle_id), "next_page_token": ""}

    @app.get("/v1/flight-reports/{report_id}")
    def get_report(report_id: str) -> dict[str, Any]:
        view = store.get(report_id)
        if view is None:
            raise HTTPException(404, "no such report")
        return {"report": view}

    return app


def __getattr__(name: str) -> Any:
    # `uvicorn flight_log_analysis.app:app` builds the app on first access, so importing
    # this module (tests) doesn't need the env config
    if name == "app":
        logging.basicConfig(level=logging.INFO)
        return create_app()
    raise AttributeError(name)
