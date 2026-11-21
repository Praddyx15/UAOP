# ICD: gRPC services

Issue 1, 2026-11-21. Between the gateway and the services. The contract is
`api/proto/uaop/gateway/v1/`; this file lists it and says what exists on each
side.

| Service | Methods | Server | Client |
|---|---|---|---|
| `VehicleCommandService` | `SubmitCommand`, `GetCommandResult` | Planned (vehicle manager) | Gateway (built; tested against a mock server) |
| `MissionService` | `CreateMission`, `GetMission`, `ListMissions`, `UploadMission`, `VerifyMission`, `CreateGeofence`, `ListGeofences`, `UploadGeofence` | Planned | None |
| `ParameterService` | `GetParameters`, `GetParameterVersions`, `WriteParameter` | Planned | None |
| `TelemetryQueryService` | `QueryHistory`, `QueryEventWindow`, `StartExport`, `GetExportStatus` | Planned | None |
| `LogService` | `ListLogs`, `GetLog`, `ExportLog`, `AnalyseLog`, `GetReport`, `ListFlightReports`, `GetFlightReport` | Planned; the analysis sidecar serves the two flight-report calls over HTTP with the same fields | None |
| `ComplianceService` | `GetPreflightChecklist`, `SignPreflightChecklist`, `GetAuditExport`, `GetRemoteIdStatus`, `GetNodeStatus` | Planned | None |

## Command contract (the critical one)

- `SubmitCommandRequest`: `vehicle_id`, `command` (`CommandKind`: ARM,
  DISARM, MODE, RTL, TAKEOFF, LAND, MISSION_START), `params` (free-form
  structure), `idempotency_key`.
- `SubmitCommandResponse`: `correlation_id`. Submission never returns the
  outcome.
- `CommandResult`: `correlation_id`, `vehicle_id`, `command`, `status`
  (PENDING, ACCEPTED, REJECTED, TIMEOUT), `reason`, `resolved_at`. TIMEOUT
  means the state is unknown and is not a failure.
- Errors: `api/proto/uaop/errors/v1/errors.proto`.

## Open points

- `params` is untyped; a typed message per command kind would let the
  contract refuse a malformed command (to consider under SAF-005).
- The request carries no actor or session; the server must take identity
  from call metadata. The gateway does not send any today (see
  `docs/lifecycle/06-design/lld-gateway.md`).
- Transport security between gateway and services is designed
  (`docs/06_Compliance_and_Quality/SECURITY.md`), not configured.
