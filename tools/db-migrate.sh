#!/usr/bin/env bash
# Postgres/TimescaleDB migration runner (M1.7, DATABASE.md §5): forward-only,
# tracked in public.schema_migrations, idempotent (already-applied files are
# skipped, never re-run). Talks to the workstation-profile Postgres via
# `docker compose exec`, matching tools/setup.sh's pattern — no local psql
# install required.
#
# Usage: tools/db-migrate.sh [up|status]

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COMPOSE_FILE="$REPO_ROOT/infrastructure/docker/compose/compose.yaml"
MIGRATIONS_DIR="$REPO_ROOT/infrastructure/postgres/migrations"
MODE="${1:-up}"

say()  { printf '\033[1;36m[db-migrate]\033[0m %s\n' "$*"; }
fail() { printf '\033[1;31m[db-migrate] FAIL:\033[0m %s\n' "$*" >&2; exit 1; }

psql_exec() {
    docker compose -f "$COMPOSE_FILE" exec -T postgres psql -U uaop -d uaop -v ON_ERROR_STOP=1 "$@"
}

ensure_tracking_table() {
    psql_exec -c "CREATE TABLE IF NOT EXISTS public.schema_migrations (
        version TEXT PRIMARY KEY,
        applied_at TIMESTAMPTZ NOT NULL DEFAULT now()
    );" >/dev/null
}

is_applied() {
    local version="$1"
    local count
    count="$(psql_exec -tA -c "SELECT count(*) FROM public.schema_migrations WHERE version = '${version}';")"
    [ "$count" = "1" ]
}

apply_migration() {
    local file="$1"
    local version
    version="$(basename "$file")"
    if is_applied "$version"; then
        say "skip (already applied): $version"
        return 0
    fi
    say "applying: $version"
    psql_exec < "$file"
    psql_exec -c "INSERT INTO public.schema_migrations (version) VALUES ('${version}');" >/dev/null
}

migrate_up() {
    ensure_tracking_table
    local file
    for file in "$MIGRATIONS_DIR"/*.sql; do
        [ -e "$file" ] || continue
        apply_migration "$file"
    done
    say "done"
}

migrate_status() {
    ensure_tracking_table
    say "applied migrations:"
    psql_exec -c "SELECT version, applied_at FROM public.schema_migrations ORDER BY version;"
}

case "$MODE" in
    up)     migrate_up ;;
    status) migrate_status ;;
    *)      fail "usage: db-migrate.sh [up|status]" ;;
esac
