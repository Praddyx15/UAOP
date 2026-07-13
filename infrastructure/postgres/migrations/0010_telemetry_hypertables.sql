-- telemetry schema (DATABASE.md §2/§4/§6, owner: svc_telemetry_engine).
-- One hypertable per TELEMETRY_ENGINE.md §2 category, column-for-column
-- matching api/proto/uaop/telemetry/v1/vehicle_telemetry.proto (M1.1) —
-- these are the persisted form of exactly what that contract carries.
-- 1h chunks (DATABASE.md §6: "keeps chunk indexes RAM-resident on RC-3"),
-- compression after 24h, continuous aggregates for the categories the GCS
-- charts most (position/attitude/battery/health) as the representative
-- pattern — the remaining categories follow identically when a real query
-- workload asks for them (TELEMETRY_ENGINE.md §4 cost-guard posture: build
-- the aggregate a query actually needs, don't pre-build all thirteen on
-- spec). esc.motors and rc.channels are JSONB/array rather than normalized
-- child tables — a deliberate v0 simplification, not an oversight.
--
-- Rollback note: DROP MATERIALIZED VIEW for each continuous aggregate, then
-- DROP TABLE for each hypertable below — destructive, verify a backup first
-- (hypertables can hold significant data volume by the time this would ever
-- be considered).

CREATE EXTENSION IF NOT EXISTS timescaledb;

-- position
CREATE TABLE IF NOT EXISTS telemetry.position (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    lat_deg DOUBLE PRECISION,
    lon_deg DOUBLE PRECISION,
    alt_wgs84_m REAL,
    alt_agl_m REAL,
    fix_type TEXT,
    satellites_visible INT,
    hdop REAL,
    vdop REAL,
    pdop REAL,
    ground_speed_mps REAL,
    course_over_ground_deg REAL,
    gps_utc_ms BIGINT,
    rtk_fix_type TEXT,
    rtk_baseline_length_m REAL,
    rtk_baseline_heading_deg REAL,
    rtk_differential_age_s REAL,
    rtk_base_station_id TEXT,
    rtk_horizontal_accuracy_m REAL,
    rtk_vertical_accuracy_m REAL
);
SELECT create_hypertable('telemetry.position', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- attitude
CREATE TABLE IF NOT EXISTS telemetry.attitude (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    roll_deg REAL,
    pitch_deg REAL,
    yaw_deg REAL,
    roll_rate_dps REAL,
    pitch_rate_dps REAL,
    yaw_rate_dps REAL,
    quat_w REAL,
    quat_x REAL,
    quat_y REAL,
    quat_z REAL
);
SELECT create_hypertable('telemetry.attitude', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- velocity
CREATE TABLE IF NOT EXISTS telemetry.velocity (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    vel_north_mps REAL,
    vel_east_mps REAL,
    vel_down_mps REAL,
    horizontal_speed_mps REAL,
    vertical_speed_mps REAL,
    airspeed_mps REAL,
    indicated_airspeed_mps REAL
);
SELECT create_hypertable('telemetry.velocity', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- imu (1000 Hz design ceiling, UAOP-HLR-030 — the highest-rate category)
CREATE TABLE IF NOT EXISTS telemetry.imu (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    instance_id INT NOT NULL DEFAULT 0,
    accel_x_mps2 REAL,
    accel_y_mps2 REAL,
    accel_z_mps2 REAL,
    gyro_x_radps REAL,
    gyro_y_radps REAL,
    gyro_z_radps REAL,
    mag_x_gauss REAL,
    mag_y_gauss REAL,
    mag_z_gauss REAL,
    temperature_c REAL,
    vibration_x REAL,
    vibration_y REAL,
    vibration_z REAL,
    clip_count_x INT,
    clip_count_y INT,
    clip_count_z INT
);
SELECT create_hypertable('telemetry.imu', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- ekf
CREATE TABLE IF NOT EXISTS telemetry.ekf (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    pos_innovation REAL,
    vel_innovation REAL,
    heading_innovation REAL,
    mag_innovation REAL,
    baro_innovation REAL,
    gps_innovation REAL,
    fault_flags BIGINT NOT NULL DEFAULT 0,
    pos_test_ratio REAL,
    vel_test_ratio REAL,
    heading_test_ratio REAL,
    mag_test_ratio REAL,
    baro_test_ratio REAL,
    gps_test_ratio REAL,
    terrain_alt_m REAL,
    wind_north_mps REAL,
    wind_east_mps REAL
);
SELECT create_hypertable('telemetry.ekf', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- battery
CREATE TABLE IF NOT EXISTS telemetry.battery (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    battery_id INT NOT NULL DEFAULT 0,
    voltage_v REAL,
    current_a REAL,
    remaining_pct REAL,
    capacity_mah REAL,
    temperature_c REAL,
    cell_count INT,
    cell_voltages_v REAL[], -- up to 14 entries, TELEMETRY_ENGINE.md §2
    fault_flags BIGINT NOT NULL DEFAULT 0,
    charge_state TEXT
);
SELECT create_hypertable('telemetry.battery', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- esc (repeated per motor -> JSONB array, v0 simplification per file header note)
CREATE TABLE IF NOT EXISTS telemetry.esc (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    motors JSONB NOT NULL -- [{index, rpm, voltage_v, current_a, temperature_c, duty_pct, failure_flags}, ...]
);
SELECT create_hypertable('telemetry.esc', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- rc
CREATE TABLE IF NOT EXISTS telemetry.rc (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    channels INT[], -- up to 18 channel values
    rssi_pct REAL,
    channel_count INT,
    signal_lost BOOLEAN NOT NULL DEFAULT false,
    failsafe_active BOOLEAN NOT NULL DEFAULT false,
    protocol TEXT
);
SELECT create_hypertable('telemetry.rc', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- link
CREATE TABLE IF NOT EXISTS telemetry.link (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    gcs_link_quality_pct REAL,
    gcs_link_latency_ms REAL,
    gcs_link_loss_pct REAL,
    data_rate_kbps REAL,
    c2_type TEXT,
    c2_quality_pct REAL,
    c2_latency_ms REAL,
    c2_loss_pct REAL
);
SELECT create_hypertable('telemetry.link', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- rf
CREATE TABLE IF NOT EXISTS telemetry.rf (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    jamming_indicator INT NOT NULL DEFAULT 0, -- 0-255
    spoofing_state TEXT,
    noise_per_ms REAL,
    interference_level REAL,
    snr_db REAL
);
SELECT create_hypertable('telemetry.rf', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- health
CREATE TABLE IF NOT EXISTS telemetry.health (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    cpu_pct REAL,
    ram_pct REAL,
    sd_free_mb REAL,
    temperature_c REAL,
    rail_5v_v REAL,
    rail_3v3_v REAL,
    error_count BIGINT NOT NULL DEFAULT 0,
    sys_status_present BIGINT NOT NULL DEFAULT 0,
    sys_status_enabled BIGINT NOT NULL DEFAULT 0,
    sys_status_health BIGINT NOT NULL DEFAULT 0
);
SELECT create_hypertable('telemetry.health', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- state (GeoPoint home_position flattened to home_lat_deg/home_lon_deg/home_alt_m)
CREATE TABLE IF NOT EXISTS telemetry.state (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    armed BOOLEAN NOT NULL DEFAULT false,
    mode TEXT,
    autopilot_type TEXT,
    vehicle_type TEXT,
    failsafe_active BOOLEAN NOT NULL DEFAULT false,
    failsafe_reason TEXT,
    landed_state TEXT,
    home_lat_deg DOUBLE PRECISION,
    home_lon_deg DOUBLE PRECISION,
    home_alt_m REAL,
    distance_home_m REAL,
    bearing_home_deg REAL,
    flight_time_s INT,
    mission_index INT,
    mission_state TEXT
);
SELECT create_hypertable('telemetry.state', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- remoteid
CREATE TABLE IF NOT EXISTS telemetry.remoteid (
    ts TIMESTAMPTZ NOT NULL,
    vehicle_id TEXT NOT NULL,
    sequence BIGINT NOT NULL,
    sim BOOLEAN NOT NULL DEFAULT false,
    uas_id TEXT,
    broadcast_active BOOLEAN NOT NULL DEFAULT false,
    uss_connected BOOLEAN NOT NULL DEFAULT false,
    last_broadcast_time TIMESTAMPTZ,
    emergency_status TEXT
);
SELECT create_hypertable('telemetry.remoteid', 'ts', chunk_time_interval => INTERVAL '1 hour', if_not_exists => TRUE);

-- Compression after 24h (DATABASE.md §6) — applied uniformly to all 13.
DO $$
DECLARE
    tbl TEXT;
BEGIN
    FOREACH tbl IN ARRAY ARRAY['position','attitude','velocity','imu','ekf','battery',
                                'esc','rc','link','rf','health','state','remoteid']
    LOOP
        EXECUTE format(
            'ALTER TABLE telemetry.%I SET (timescaledb.compress, timescaledb.compress_segmentby = %L);',
            tbl, 'vehicle_id'
        );
        PERFORM add_compression_policy(format('telemetry.%I', tbl), INTERVAL '24 hours', if_not_exists => TRUE);
    END LOOP;
END
$$;

-- Continuous aggregates (1s/1min) for the categories the GCS charts most
-- (TELEMETRY_ENGINE.md §4) — representative pattern; add more per-category
-- as real query load asks for them.
CREATE MATERIALIZED VIEW IF NOT EXISTS telemetry.position_1min
WITH (timescaledb.continuous) AS
SELECT vehicle_id,
       time_bucket('1 minute', ts) AS bucket,
       avg(lat_deg) AS avg_lat_deg,
       avg(lon_deg) AS avg_lon_deg,
       avg(alt_agl_m) AS avg_alt_agl_m,
       avg(ground_speed_mps) AS avg_ground_speed_mps
FROM telemetry.position
GROUP BY vehicle_id, bucket
WITH NO DATA;

CREATE MATERIALIZED VIEW IF NOT EXISTS telemetry.attitude_1min
WITH (timescaledb.continuous) AS
SELECT vehicle_id,
       time_bucket('1 minute', ts) AS bucket,
       avg(roll_deg) AS avg_roll_deg,
       avg(pitch_deg) AS avg_pitch_deg,
       avg(yaw_deg) AS avg_yaw_deg
FROM telemetry.attitude
GROUP BY vehicle_id, bucket
WITH NO DATA;

CREATE MATERIALIZED VIEW IF NOT EXISTS telemetry.battery_1min
WITH (timescaledb.continuous) AS
SELECT vehicle_id,
       time_bucket('1 minute', ts) AS bucket,
       avg(voltage_v) AS avg_voltage_v,
       avg(remaining_pct) AS avg_remaining_pct,
       min(remaining_pct) AS min_remaining_pct
FROM telemetry.battery
GROUP BY vehicle_id, bucket
WITH NO DATA;

CREATE MATERIALIZED VIEW IF NOT EXISTS telemetry.health_1min
WITH (timescaledb.continuous) AS
SELECT vehicle_id,
       time_bucket('1 minute', ts) AS bucket,
       avg(cpu_pct) AS avg_cpu_pct,
       avg(ram_pct) AS avg_ram_pct,
       max(error_count) AS max_error_count
FROM telemetry.health
GROUP BY vehicle_id, bucket
WITH NO DATA;

GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA telemetry TO svc_telemetry_engine;
