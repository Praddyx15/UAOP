-- node schema (DATABASE.md §2). Same ownership gap as auth: no
-- node-supervisor service exists in PROJECT_STRUCTURE.md yet, so no svc_*
-- role is granted here. deployment_records is a query-optimized companion
-- to the generic audit.chain (DEPLOYMENT.md §6: "the first entry in that
-- node's audit chain") — this table exists so deployment history can be
-- queried directly without scanning the chain's JSONB events, not as a
-- replacement for the chain's tamper-evidence role.
--
-- Rollback note: DROP TABLE node.deployment_records, node.settings —
-- destructive, verify a backup first.

CREATE TABLE IF NOT EXISTS node.settings (
    key TEXT PRIMARY KEY,
    value JSONB NOT NULL,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS node.deployment_records (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    version TEXT NOT NULL,
    smoke_suite_passed BOOLEAN NOT NULL,
    signature BYTEA,
    details JSONB,
    deployed_at TIMESTAMPTZ NOT NULL DEFAULT now()
);
