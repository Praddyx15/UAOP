-- flightlog schema (DATABASE.md §2, owner: svc_flight_log_engine).
-- "metadata; blobs in MinIO" — this table never stores log bytes, only the
-- content hash (tamper evidence, UAOP-HLR-032) and a pointer to the MinIO
-- object; integrity is verified by re-hashing the blob on read
-- (MICROSERVICES.md flight-log-engine), not by trusting this row.
--
-- Rollback note: DROP TABLE flightlog.reports, flightlog.entries —
-- destructive, verify a backup first (and that the referenced MinIO objects
-- are handled separately — this migration has no opinion on blob storage).

CREATE TABLE IF NOT EXISTS flightlog.entries (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    vehicle_id TEXT NOT NULL,
    content_hash TEXT NOT NULL, -- tamper evidence, UAOP-HLR-032
    size_bytes BIGINT NOT NULL,
    minio_key TEXT NOT NULL,
    captured_at TIMESTAMPTZ NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_flightlog_entries_vehicle ON flightlog.entries(vehicle_id, captured_at);

CREATE TABLE IF NOT EXISTS flightlog.reports (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    log_id UUID NOT NULL REFERENCES flightlog.entries(id),
    pdf_minio_key TEXT,
    generated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA flightlog TO svc_flight_log_engine;
