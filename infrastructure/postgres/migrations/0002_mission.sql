-- mission schema (DATABASE.md §2, owner: svc_mission_engine). Plan content
-- (waypoints/actions/DO_LAND_START/RTL) stays JSONB here just as it does in
-- api/proto/uaop/gateway/v1/mission.proto's google.protobuf.Struct — the
-- typed domain model is mission-engine's own contract to define when that
-- engine is actually built (M2+), not frozen prematurely at the schema layer.
--
-- Rollback note: DROP TABLE mission.airspace_imports, mission.geofences,
-- mission.deployments, mission.plan_versions, mission.plans (in that order,
-- respecting FKs) — destructive, verify a backup first.

CREATE TABLE IF NOT EXISTS mission.plans (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    plan JSONB NOT NULL,
    plan_hash TEXT NOT NULL, -- content hash, UAOP-HLR-012 mission identity
    created_by TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS mission.plan_versions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    plan_id UUID NOT NULL REFERENCES mission.plans(id),
    version INT NOT NULL,
    plan JSONB NOT NULL,
    plan_hash TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    UNIQUE (plan_id, version)
);

CREATE TABLE IF NOT EXISTS mission.deployments (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    plan_id UUID NOT NULL REFERENCES mission.plans(id),
    vehicle_id TEXT NOT NULL,
    correlation_id TEXT NOT NULL,
    state TEXT NOT NULL DEFAULT 'PENDING'
        CHECK (state IN ('PENDING', 'UPLOADED', 'VERIFIED', 'MISMATCH', 'FAILED')),
    uploaded_at TIMESTAMPTZ,
    verified_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS idx_mission_deployments_vehicle ON mission.deployments(vehicle_id);

CREATE TABLE IF NOT EXISTS mission.geofences (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    kind TEXT NOT NULL CHECK (kind IN ('INCLUSION', 'EXCLUSION')),
    breach_action TEXT NOT NULL
        CHECK (breach_action IN ('RTL', 'LAND', 'LOITER', 'HOLD', 'REPORT')),
    geometry JSONB NOT NULL, -- polygon or circle definition
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS mission.airspace_imports (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    source TEXT NOT NULL,
    data JSONB NOT NULL,
    imported_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA mission TO svc_mission_engine;
