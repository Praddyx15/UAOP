-- audit schema (DATABASE.md §3, COMPLIANCE.md §B.3): append-only,
-- hash-chained (hash_n = SHA256(hash_{n-1} || canonical(event_n))),
-- single-writer (compliance-engine). Monthly partitions keep the append-only
-- table's indexes manageable at volume; the chain query itself (ORDER BY seq)
-- works transparently across partitions.
--
-- Chain integrity is enforced two ways, deliberately redundant:
--   1. DB role grants: compliance-engine gets SELECT+INSERT only — UPDATE,
--      DELETE, TRUNCATE are never granted (DATA_FLOW.md §5's "INSERT-only
--      role" — the query side needs SELECT for compliance-engine's own
--      export/verify API, so "insert-only" means insert-only with respect
--      to mutation, not literally no reads).
--   2. A BEFORE INSERT trigger that refuses a row whose prev_hash doesn't
--      match the current chain head — the DB itself catches an application
--      bug that would otherwise silently break the chain, rather than
--      relying solely on the offline verifier job to notice later.
--
-- Rollback note: DROP TRIGGER trg_audit_chain_integrity, DROP FUNCTION
-- audit.check_chain_integrity, DROP TABLE audit.exports, DROP TABLE
-- audit.chain (and its partitions) — destructive; an append-only evidence
-- table should essentially never be rolled back in practice.

CREATE TABLE IF NOT EXISTS audit.chain (
    seq BIGINT GENERATED ALWAYS AS IDENTITY,
    prev_hash BYTEA,
    event JSONB NOT NULL,
    hash BYTEA NOT NULL,
    actor TEXT,
    vehicle_id TEXT,
    correlation_id TEXT,
    occurred_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (seq, occurred_at)
) PARTITION BY RANGE (occurred_at);

-- Baseline partitions: current month + next, so the table is writable
-- immediately. Automated rollover (a scheduled `SELECT
-- audit.create_monthly_partition(...)` job) is DEVOPS/operational tooling,
-- not solved by this migration baseline.
CREATE OR REPLACE FUNCTION audit.create_monthly_partition(for_month DATE)
RETURNS void AS $$
DECLARE
    partition_name TEXT := 'chain_' || to_char(for_month, 'YYYY_MM');
    start_of_month DATE := date_trunc('month', for_month);
    start_of_next_month DATE := start_of_month + INTERVAL '1 month';
BEGIN
    EXECUTE format(
        'CREATE TABLE IF NOT EXISTS audit.%I PARTITION OF audit.chain FOR VALUES FROM (%L) TO (%L);',
        partition_name, start_of_month, start_of_next_month
    );
END;
$$ LANGUAGE plpgsql;

SELECT audit.create_monthly_partition(CURRENT_DATE);
SELECT audit.create_monthly_partition((CURRENT_DATE + INTERVAL '1 month')::DATE);

CREATE OR REPLACE FUNCTION audit.check_chain_integrity() RETURNS TRIGGER AS $$
DECLARE
    expected_prev_hash BYTEA;
BEGIN
    SELECT hash INTO expected_prev_hash FROM audit.chain ORDER BY seq DESC LIMIT 1;
    IF expected_prev_hash IS NULL THEN
        IF NEW.prev_hash IS NOT NULL THEN
            RAISE EXCEPTION 'audit chain integrity violation: first entry must have NULL prev_hash';
        END IF;
    ELSIF NEW.prev_hash IS DISTINCT FROM expected_prev_hash THEN
        RAISE EXCEPTION 'audit chain integrity violation: prev_hash does not match current chain head';
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS trg_audit_chain_integrity ON audit.chain;
CREATE TRIGGER trg_audit_chain_integrity
    BEFORE INSERT ON audit.chain
    FOR EACH ROW EXECUTE FUNCTION audit.check_chain_integrity();

CREATE TABLE IF NOT EXISTS audit.exports (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    requested_by TEXT,
    from_ts TIMESTAMPTZ,
    to_ts TIMESTAMPTZ,
    domain TEXT,
    chain_verified BOOLEAN NOT NULL,
    chain_head_hash BYTEA,
    download_url TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

GRANT SELECT, INSERT ON audit.chain TO svc_compliance_engine;
REVOKE UPDATE, DELETE, TRUNCATE ON audit.chain FROM svc_compliance_engine;
GRANT SELECT, INSERT, UPDATE, DELETE ON audit.exports TO svc_compliance_engine;
