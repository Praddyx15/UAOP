"""Client for FlightMD's tune-fly-verify endpoint: POST /airframe/{label}/changes.

FlightMD here is a self-hosted instance on the node or the operator's LAN, never the
public deployment by default (ADR-0022). Stdlib only, so the sidecar adds no HTTP
client dependency for one call.
"""

import json
import urllib.error
import urllib.request
from typing import Any
from urllib.parse import quote


class _NoRedirect(urllib.request.HTTPRedirectHandler):
    # urllib would resend X-Airframe-Token to wherever a redirect points
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        return None


_opener = urllib.request.build_opener(_NoRedirect)


class FlightmdError(Exception):
    def __init__(self, message: str, *, status: int | None = None, transient: bool = False) -> None:
        super().__init__(message)
        self.status = status
        self.transient = transient


class FlightmdClient:
    def __init__(self, base_url: str, tokens: dict[str, str], timeout_s: float = 10.0) -> None:
        self.base_url = base_url.rstrip("/")
        self._tokens = tokens
        self.timeout_s = timeout_s

    @property
    def enabled(self) -> bool:
        return bool(self.base_url)

    def record_changes(
        self,
        airframe_label: str,
        report_id: str,
        changes: list[dict[str, Any]],
        note: str | None = None,
    ) -> dict[str, Any]:
        token = self._tokens.get(airframe_label)
        if not token:
            raise FlightmdError(f"no FlightMD token configured for airframe {airframe_label!r}")
        body: dict[str, Any] = {"report_id": report_id, "changes": changes, "applied_by": "uaop"}
        if note:
            body["note"] = note
        req = urllib.request.Request(
            f"{self.base_url}/airframe/{quote(airframe_label, safe='')}/changes",
            data=json.dumps(body).encode("utf-8"),
            method="POST",
            headers={"Content-Type": "application/json", "X-Airframe-Token": token},
        )
        try:
            with _opener.open(req, timeout=self.timeout_s) as resp:
                return json.loads(resp.read().decode("utf-8") or "{}")
        except urllib.error.HTTPError as exc:
            # 429 and 5xx are worth a redelivery; anything else will fail the same way again
            transient = exc.code == 429 or exc.code >= 500
            raise FlightmdError(f"FlightMD answered {exc.code}", status=exc.code, transient=transient) from None
        except (urllib.error.URLError, TimeoutError) as exc:
            raise FlightmdError(f"FlightMD unreachable: {exc}", transient=True) from None
