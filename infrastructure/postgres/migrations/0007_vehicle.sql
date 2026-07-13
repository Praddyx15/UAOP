-- vehicle schema (DATABASE.md §2, owner: svc_vehicle_manager). registry is
-- the durable identity record; profiles is per-vehicle capability/config
-- data (airframe type, autopilot quirks) kept separate from the hot
-- session/state path (which lives in Redis per DATABASE.md §1, not here —
-- this table is for what's true about a vehicle across restarts, not its
-- live connection state).
--
-- Rollback note: DROP TABLE vehicle.profiles, vehicle.registry —
-- destructive, verify a backup first.

CREATE TABLE IF NOT EXISTS vehicle.registry (
    vehicle_id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    autopilot_type TEXT CHECK (autopilot_type IN ('PX4', 'ARDUPILOT', 'OTHER')),
    vehicle_type TEXT CHECK (vehicle_type IN
        ('MULTIROTOR', 'FIXED_WING', 'VTOL', 'HELICOPTER', 'GROUND_ROVER', 'OTHER')),
    registered_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS vehicle.profiles (
    vehicle_id TEXT PRIMARY KEY REFERENCES vehicle.registry(vehicle_id),
    profile JSONB NOT NULL DEFAULT '{}'::jsonb,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA vehicle TO svc_vehicle_manager;
