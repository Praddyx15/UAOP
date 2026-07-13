-- auth schema (DATABASE.md §2). Ownership is genuinely undecided: no
-- auth-service/identity-service exists in PROJECT_STRUCTURE.md's service
-- list as of M1 — api-gateway (M1.6) issues and verifies JWTs but the local
-- identity provider's user store (SECURITY.md §3: Argon2id-hashed
-- credentials) doesn't exist yet, hence api-gateway's dev-only login route.
-- Tables created here so the schema exists to build against; no svc_* role
-- is granted anything here (0001's role list deliberately has no
-- svc_auth_* entry) — that grant lands once ownership is decided, not
-- guessed at in this migration.
--
-- Rollback note: DROP TABLE auth.tokens, auth.roles, auth.users —
-- destructive, verify a backup first.

CREATE TABLE IF NOT EXISTS auth.users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL, -- Argon2id, SECURITY.md §3/§4
    org TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

-- Six-role RBAC (SECURITY.md §3); a user may hold more than one role,
-- possibly scoped per-org, hence a mapping table rather than a single column.
CREATE TABLE IF NOT EXISTS auth.roles (
    user_id UUID NOT NULL REFERENCES auth.users(id),
    role TEXT NOT NULL CHECK (role IN
        ('PLATFORM_ADMIN', 'ORG_ADMIN', 'FLEET_MANAGER', 'PILOT', 'OBSERVER', 'MAINTENANCE')),
    org TEXT,
    PRIMARY KEY (user_id, role, org)
);

-- Persistent record of issued tokens (distinct from the Redis-backed
-- denylist in backend/api-gateway/src/auth/denylist.js, which is for
-- revocation lookups only, not an audit trail of what was ever issued).
CREATE TABLE IF NOT EXISTS auth.tokens (
    jti UUID PRIMARY KEY,
    user_id UUID NOT NULL REFERENCES auth.users(id),
    issued_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    expires_at TIMESTAMPTZ NOT NULL
);
