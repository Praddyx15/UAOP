#!/usr/bin/env bash
# UAOP workstation setup v0 (DEPLOYMENT.md §2, M0.6).
# Target: clean clone -> ./tools/setup.sh -> infra up + build green in <= 15 min.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COMPOSE_FILE="$REPO_ROOT/infrastructure/docker/compose/compose.yaml"
MODE="${1:-up}"

say()  { printf '\033[1;36m[uaop]\033[0m %s\n' "$*"; }
fail() { printf '\033[1;31m[uaop] FAIL:\033[0m %s\n' "$*" >&2; exit 1; }

check_prereqs() {
    say "checking prerequisites"
    command -v docker >/dev/null 2>&1 || fail "docker not found — install Docker Desktop / docker-ce"
    docker info >/dev/null 2>&1     || fail "docker engine not running"
    command -v cmake >/dev/null 2>&1 || say "warning: cmake not found — infra will start, services won't build"
    command -v python3 >/dev/null 2>&1 || command -v python >/dev/null 2>&1 || say "warning: python not found — CI guards won't run locally"
}

infra_up() {
    say "starting core infrastructure (NATS, TimescaleDB, Redis, MinIO)"
    docker compose -f "$COMPOSE_FILE" up -d --wait
    say "infrastructure healthy:"
    docker compose -f "$COMPOSE_FILE" ps --format 'table {{.Service}}\t{{.Status}}'
}

sitl_up() {
    say "starting PX4 SITL (headless Gazebo) — MAVLink on udp:14550"
    docker compose -f "$COMPOSE_FILE" --profile sitl up -d --wait
}

build_services() {
    if command -v cmake >/dev/null 2>&1; then
        say "building services (debug preset)"
        (cd "$REPO_ROOT" && cmake --preset debug && cmake --build --preset debug && ctest --preset debug)
    fi
}

guards() {
    local py; py="$(command -v python3 || command -v python)" || return 0
    say "running CI guards locally"
    "$py" "$REPO_ROOT/tools/ci/check_frozen_dirs.py"
    "$py" "$REPO_ROOT/tools/ci/check_doc_links.py"
    "$py" "$REPO_ROOT/tools/ci/check_licenses.py"
    "$py" "$REPO_ROOT/compliance/tools/rtm-generator/rtm_gen.py" --check
}

case "$MODE" in
    up)      check_prereqs; infra_up; build_services; guards; say "workstation ready" ;;
    sitl)    check_prereqs; infra_up; sitl_up; say "workstation + SITL ready" ;;
    down)    docker compose -f "$COMPOSE_FILE" --profile sitl down; say "infrastructure stopped" ;;
    status)  docker compose -f "$COMPOSE_FILE" ps ;;
    *)       fail "usage: setup.sh [up|sitl|down|status]" ;;
esac
