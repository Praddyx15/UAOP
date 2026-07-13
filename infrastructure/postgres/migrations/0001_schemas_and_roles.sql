-- Baseline: one schema per DATABASE.md §2's census, one role per owning
-- service per DATA_FLOW.md §5's ownership matrix. Cross-service access is
-- always by API/event, never by SQL (DATA_FLOW.md §5) — enforced here by
-- simply never granting a second service's role anything on a schema it
-- doesn't own, rather than by a more elaborate row-level policy.
--
-- Rollback note (forward-only execution; DATABASE.md §5): DROP SCHEMA ...
-- CASCADE for each schema below, DROP ROLE for each role — destructive,
-- never run against a schema with data without a verified backup first.

-- gen_random_uuid() is core since PG13, but pgcrypto is harmless to ensure
-- explicitly rather than assume the exact core-function boundary.
CREATE EXTENSION IF NOT EXISTS pgcrypto;

CREATE SCHEMA IF NOT EXISTS mission;
CREATE SCHEMA IF NOT EXISTS param;
CREATE SCHEMA IF NOT EXISTS audit;
CREATE SCHEMA IF NOT EXISTS compliance;
CREATE SCHEMA IF NOT EXISTS flightlog;
CREATE SCHEMA IF NOT EXISTS vehicle;
CREATE SCHEMA IF NOT EXISTS telemetry;

-- auth.* and node.* are in DATABASE.md §2's census but have no owning
-- service in DATA_FLOW.md §5's matrix yet (no auth-service or
-- node-supervisor exists in PROJECT_STRUCTURE.md's service list as of M1) —
-- schemas created so the namespace is reserved, roles deliberately deferred
-- until ownership is decided rather than guessed.
CREATE SCHEMA IF NOT EXISTS auth;
CREATE SCHEMA IF NOT EXISTS node;

DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'svc_mission_engine') THEN
        CREATE ROLE svc_mission_engine NOLOGIN;
    END IF;
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'svc_parameter_engine') THEN
        CREATE ROLE svc_parameter_engine NOLOGIN;
    END IF;
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'svc_compliance_engine') THEN
        CREATE ROLE svc_compliance_engine NOLOGIN;
    END IF;
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'svc_flight_log_engine') THEN
        CREATE ROLE svc_flight_log_engine NOLOGIN;
    END IF;
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'svc_vehicle_manager') THEN
        CREATE ROLE svc_vehicle_manager NOLOGIN;
    END IF;
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'svc_telemetry_engine') THEN
        CREATE ROLE svc_telemetry_engine NOLOGIN;
    END IF;
END
$$;

-- NOLOGIN roles above are permission groups; the actual per-service
-- connection user (created at deployment time, out of this repo's baseline
-- per SECURITY.md — no credentials belong in version control) is GRANTed
-- its matching svc_* role rather than owning grants directly.

GRANT USAGE ON SCHEMA mission TO svc_mission_engine;
GRANT USAGE ON SCHEMA param TO svc_parameter_engine;
GRANT USAGE ON SCHEMA compliance TO svc_compliance_engine;
GRANT USAGE ON SCHEMA flightlog TO svc_flight_log_engine;
GRANT USAGE ON SCHEMA vehicle TO svc_vehicle_manager;
GRANT USAGE ON SCHEMA telemetry TO svc_telemetry_engine;
-- audit: compliance-engine is the sole writer, INSERT-only (DATA_FLOW.md §5,
-- DATABASE.md §3) — granted at table level in 0004_audit.sql, not schema-wide,
-- since USAGE alone doesn't imply any DML right.
GRANT USAGE ON SCHEMA audit TO svc_compliance_engine;
