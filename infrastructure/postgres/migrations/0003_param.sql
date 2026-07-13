-- param schema (DATABASE.md §2, owner: svc_parameter_engine). metadata_packs
-- is where seeded parameter metadata lives (e.g. the FlightMD-derived PX4
-- defaults/safe-ranges under backend/services/parameter-engine/data/,
-- ADR-0018) once loaded; pending_writes exists because writes are confirmed
-- via MAVLink ACK + read-back before being considered final
-- (UAOP-HLR-020/021) — a write is "pending" the whole time that's in flight.
--
-- Rollback note: DROP TABLE param.pending_writes, param.versions,
-- param.metadata_packs — destructive, verify a backup first.

CREATE TABLE IF NOT EXISTS param.metadata_packs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    source TEXT NOT NULL,
    data JSONB NOT NULL, -- per-parameter: group, type, min/max, description
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS param.versions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    vehicle_id TEXT NOT NULL,
    name TEXT NOT NULL,
    old_value JSONB,
    new_value JSONB NOT NULL,
    operator_id TEXT NOT NULL,
    risk_class TEXT NOT NULL DEFAULT 'LOW' CHECK (risk_class IN ('LOW', 'MEDIUM', 'HIGH')),
    changed_at TIMESTAMPTZ NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS idx_param_versions_vehicle_name ON param.versions(vehicle_id, name);

CREATE TABLE IF NOT EXISTS param.pending_writes (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    vehicle_id TEXT NOT NULL,
    name TEXT NOT NULL,
    requested_value JSONB NOT NULL,
    correlation_id TEXT NOT NULL UNIQUE,
    status TEXT NOT NULL DEFAULT 'PENDING' CHECK (status IN ('PENDING', 'CONFIRMED', 'FAILED')),
    requested_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    resolved_at TIMESTAMPTZ
);

GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA param TO svc_parameter_engine;
