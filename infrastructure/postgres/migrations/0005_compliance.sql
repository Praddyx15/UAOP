-- compliance schema (DATABASE.md §2, owner: svc_compliance_engine). The
-- 30-item pre-flight checklist gating AUTHORISE FLIGHT (UAOP-HLR-043,
-- COMPLIANCE.md §B.2) — signoffs is a separate table from checklist_runs so
-- the binding of operator identity to a signature is an explicit,
-- independently-queryable event, not just a column flip on the run row.
--
-- Rollback note: DROP TABLE compliance.signoffs, compliance.checklist_runs,
-- compliance.checklist_templates — destructive, verify a backup first.

CREATE TABLE IF NOT EXISTS compliance.checklist_templates (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    items JSONB NOT NULL, -- [{item_id, description}, ...] — 30 items per COMPLIANCE.md §B.2
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS compliance.checklist_runs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    template_id UUID NOT NULL REFERENCES compliance.checklist_templates(id),
    vehicle_id TEXT NOT NULL,
    items JSONB NOT NULL, -- [{item_id, description, checked}, ...]
    state TEXT NOT NULL DEFAULT 'IN_PROGRESS' CHECK (state IN ('IN_PROGRESS', 'SIGNED')),
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS idx_compliance_checklist_runs_vehicle ON compliance.checklist_runs(vehicle_id);

CREATE TABLE IF NOT EXISTS compliance.signoffs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    checklist_run_id UUID NOT NULL REFERENCES compliance.checklist_runs(id),
    signed_by TEXT NOT NULL, -- operator identity, bound at signing
    signed_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA compliance TO svc_compliance_engine;
